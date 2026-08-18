// SPDX-License-Identifier: LGPL-3.0-or-later
// CD-ROM raw-sector EDC/ECC helpers for the Makou Reactor ff7tk overlay.
#pragma once

#include <array>
#include <cstring>

namespace ff7tk_psx_cd {

constexpr int Mode1EdcOffset = 2064;
constexpr int Mode2Form1EdcOffset = 2072;
constexpr int Mode2Form2EdcOffset = 2348;
constexpr int EccPOffset = 2076;
constexpr int EccQOffset = 2248;

inline const std::array<quint32, 256> &edcTable()
{
    static const std::array<quint32, 256> table = []() {
        std::array<quint32, 256> result{};
        for (quint32 i = 0; i < 256; ++i) {
            quint32 value = i;
            for (int bit = 0; bit < 8; ++bit) {
                value = (value >> 1) ^ ((value & 1U) ? 0xD8018001U : 0U);
            }
            result[i] = value;
        }
        return result;
    }();
    return table;
}

inline quint32 computeEdc(const quint8 *data, qsizetype size)
{
    const auto &table = edcTable();
    quint32 edc = 0;
    for (qsizetype i = 0; i < size; ++i) {
        edc = (edc >> 8) ^ table[(edc ^ data[i]) & 0xffU];
    }
    return edc;
}

struct EccTables {
    std::array<quint8, 256> forward{};
    std::array<quint8, 256> backward{};

    EccTables()
    {
        for (int i = 0; i < 256; ++i) {
            const int doubled = (i << 1) ^ ((i & 0x80) ? 0x11d : 0);
            forward[i] = quint8(doubled);
            backward[i ^ doubled] = quint8(i);
        }
    }
};

inline const EccTables &eccTables()
{
    static const EccTables tables;
    return tables;
}

inline void computeEcc(const quint8 *source, quint32 majorCount, quint32 minorCount,
                       quint32 majorMultiplier, quint32 minorIncrement, quint8 *destination)
{
    const auto &tables = eccTables();
    const quint32 size = majorCount * minorCount;

    for (quint32 major = 0; major < majorCount; ++major) {
        quint32 index = (major >> 1) * majorMultiplier + (major & 1U);
        quint8 eccA = 0;
        quint8 eccB = 0;

        for (quint32 minor = 0; minor < minorCount; ++minor) {
            const quint8 value = source[index];
            index += minorIncrement;
            if (index >= size) {
                index -= size;
            }
            eccA ^= value;
            eccB ^= value;
            eccA = tables.forward[eccA];
        }

        eccA = tables.backward[tables.forward[eccA] ^ eccB];
        destination[major] = eccA;
        destination[major + majorCount] = eccA ^ eccB;
    }
}

inline void writeEdcLittleEndian(quint8 *destination, quint32 edc)
{
    destination[0] = quint8(edc);
    destination[1] = quint8(edc >> 8);
    destination[2] = quint8(edc >> 16);
    destination[3] = quint8(edc >> 24);
}

inline bool hasCdRomSync(const quint8 *sector)
{
    if (sector[0] != 0 || sector[11] != 0) {
        return false;
    }
    for (int i = 1; i < 11; ++i) {
        if (sector[i] != 0xff) {
            return false;
        }
    }
    return true;
}

// Recompute integrity fields for a sector whose payload has actually changed.
// Untouched sectors are never passed through this function by the ff7tk changes.
inline bool updateSectorEdcEcc(QByteArray &sectorData)
{
    if (sectorData.size() != SECTOR_SIZE) {
        return false;
    }

    auto *sector = reinterpret_cast<quint8 *>(sectorData.data());
    if (!hasCdRomSync(sector)) {
        return true; // Audio/non-data: nothing to regenerate.
    }

    const quint8 mode = sector[15];
    if (mode == 1) {
        const quint32 edc = computeEdc(sector, Mode1EdcOffset);
        writeEdcLittleEndian(sector + Mode1EdcOffset, edc);
        std::memset(sector + Mode1EdcOffset + 4, 0, 8);
        computeEcc(sector + 12, 86, 24, 2, 86, sector + EccPOffset);
        computeEcc(sector + 12, 52, 43, 86, 88, sector + EccQOffset);
        return true;
    }

    if (mode != 2) {
        return true;
    }

    // XA repeats the four-byte subheader at 16..19 and 20..23.
    if (std::memcmp(sector + 16, sector + 20, 4) != 0) {
        return true; // Plain Mode 2, not XA Form 1/2.
    }

    // XA Mode 2 Form 2: 2324 user bytes + EDC, no P/Q ECC.
    if ((sector[18] & 0x20U) != 0) {
        const quint32 edc = computeEdc(sector + 16, Mode2Form2EdcOffset - 16);
        writeEdcLittleEndian(sector + Mode2Form2EdcOffset, edc);
        return true;
    }

    // XA Mode 2 Form 1: EDC covers bytes 16..2071.
    const quint32 edc = computeEdc(sector + 16, Mode2Form1EdcOffset - 16);
    writeEdcLittleEndian(sector + Mode2Form1EdcOffset, edc);

    // P/Q ECC is position-independent for XA Form 1: bytes 12..15 are
    // treated as zero while parity is calculated.
    quint8 addressAndMode[4];
    std::memcpy(addressAndMode, sector + 12, sizeof(addressAndMode));
    std::memset(sector + 12, 0, sizeof(addressAndMode));
    computeEcc(sector + 12, 86, 24, 2, 86, sector + EccPOffset);
    computeEcc(sector + 12, 52, 43, 86, 88, sector + EccQOffset);
    std::memcpy(sector + 12, addressAndMode, sizeof(addressAndMode));

    return true;
}

inline bool repairSector(IsoArchiveIO &io, quint32 num)
{
    const qint64 previousPos = io.pos();
    const qint64 sectorPos = qint64(num) * SECTOR_SIZE;

    if (!io.seek(sectorPos)) {
        return false;
    }

    QByteArray sectorData = io.read(SECTOR_SIZE);
    bool ok = sectorData.size() == SECTOR_SIZE && updateSectorEdcEcc(sectorData);
    if (ok) {
        ok = io.seek(sectorPos) && io.write(sectorData) == SECTOR_SIZE;
    }

    if (!io.seek(previousPos)) {
        ok = false;
    }
    return ok;
}

// Relocate an existing raw sector. For PS1 XA Mode 2 sectors the EDC/ECC does
// not depend on the MSF address, so preserve its original integrity bytes
// exactly (including any intentional anomaly). Mode 1 does include the address
// in EDC, so it must be regenerated after moving.
inline bool writeRelocatedSector(IsoArchiveIO &io, QByteArray sectorData)
{
    if (sectorData.size() != SECTOR_SIZE || io.pos() % SECTOR_SIZE != 0) {
        return false;
    }

    auto *sector = reinterpret_cast<quint8 *>(sectorData.data());
    if (hasCdRomSync(sector) && (sector[15] == 1 || sector[15] == 2)) {
        const quint8 mode = sector[15];
        const QByteArray address = IsoArchiveIO::int2Header(io.currentSector());
        if (address.size() != 3) {
            return false;
        }
        std::memcpy(sector + 12, address.constData(), 3);

        if (mode == 1 && !updateSectorEdcEcc(sectorData)) {
            return false;
        }
    }

    return io.write(sectorData) == SECTOR_SIZE;
}

} // namespace ff7tk_psx_cd
