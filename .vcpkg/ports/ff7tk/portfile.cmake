if(VCPKG_TARGET_IS_WINDOWS)
  set(VCPKG_C_FLAGS "-bigobj ${VCPKG_C_FLAGS}")
  set(VCPKG_CXX_FLAGS "-bigobj ${VCPKG_CXX_FLAGS}")
endif()

vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO sithlord48/ff7tk
  REF v1.3.1
  SHA512 4800bfaa50d5dc471b703b2c09f45da3478ba7dbc78625d55214069d3ec7acc898663934a0c5195e2170a98dfd3c32299198eee1e675936301cc72a528e289ed
  HEAD_REF master
)

# Replace a complete source region using symbol markers rather than matching
# the original function body. This is deliberately insensitive to CRLF/LF and
# harmless upstream whitespace differences.
function(_mr_replace_region FILE_PATH START_MARKER END_MARKER REPLACEMENT)
  file(READ "${FILE_PATH}" _text)
  string(FIND "${_text}" "${START_MARKER}" _start)
  if(_start EQUAL -1)
    message(FATAL_ERROR "ff7tk PSX fix: start marker not found: ${START_MARKER}")
  endif()

  string(FIND "${_text}" "${END_MARKER}" _end)
  if(_end EQUAL -1 OR _end LESS _start)
    message(FATAL_ERROR "ff7tk PSX fix: end marker not found after start: ${END_MARKER}")
  endif()

  string(LENGTH "${_text}" _text_len)
  math(EXPR _suffix_len "${_text_len} - ${_end}")
  string(SUBSTRING "${_text}" 0 ${_start} _prefix)
  string(SUBSTRING "${_text}" ${_end} ${_suffix_len} _suffix)
  file(WRITE "${FILE_PATH}" "${_prefix}${REPLACEMENT}${_suffix}")
endfunction()

set(_iso_cpp "${SOURCE_PATH}/src/formats/IsoArchive.cpp")
file(COPY "${CMAKE_CURRENT_LIST_DIR}/psx-cdrom-edc-ecc.h"
     DESTINATION "${SOURCE_PATH}/src/formats")

# Add the helper include without depending on source line endings.
file(READ "${_iso_cpp}" _iso_text)
string(FIND "${_iso_text}" "#include \"psx-cdrom-edc-ecc.h\"" _helper_include)
if(_helper_include EQUAL -1)
  string(FIND "${_iso_text}" "#include <QtEndian>" _qtendian_include)
  if(_qtendian_include EQUAL -1)
    message(FATAL_ERROR "ff7tk PSX fix: QtEndian include marker not found")
  endif()
  string(REPLACE "#include <QtEndian>"
                 "#include <QtEndian>\n#include \"psx-cdrom-edc-ecc.h\""
                 _iso_text "${_iso_text}")
  file(WRITE "${_iso_cpp}" "${_iso_text}")
endif()

_mr_replace_region("${_iso_cpp}"
  "qint64 IsoArchiveIO::writeIso(const char *data, qint64 maxSize)"
  "qint64 IsoArchiveIO::writeIso(const QByteArray &byteArray)"
[=[qint64 IsoArchiveIO::writeIso(const char *data, qint64 maxSize)
{
    qint64 write = 0, writeTotal = 0, seqLen;

    if (!seekIso(isoPos(pos()))) {
        return 0;
    }

    const qint64 startIsoPos = posIso();

    seqLen = std::min(SECTOR_SIZE_HEADER + SECTOR_SIZE_DATA - (pos() % SECTOR_SIZE), maxSize);
    while ((write = this->write(data, seqLen)) > 0) {
        data += write;
        maxSize -= write;
        writeTotal += write;
        seqLen = std::min(qint64(SECTOR_SIZE_DATA), maxSize);
        if (pos() % SECTOR_SIZE >= SECTOR_SIZE_HEADER + SECTOR_SIZE_DATA
                && !seek(pos() + SECTOR_SIZE_HEADER + SECTOR_SIZE_FOOTER)) {
            break;
        }
    }

    if (writeTotal > 0) {
        const quint32 firstSector = quint32(startIsoPos / SECTOR_SIZE_DATA);
        const quint32 lastSector = quint32((startIsoPos + writeTotal - 1) / SECTOR_SIZE_DATA);
        for (quint32 sector = firstSector; sector <= lastSector; ++sector) {
            if (!ff7tk_psx_cd::repairSector(*this, sector)) {
                return -1;
            }
        }
    }

    return write < 0 ? write : writeTotal;
}
]=])

_mr_replace_region("${_iso_cpp}"
  "bool IsoArchiveIO::writeSector(const QByteArray &data, quint8 type, quint8 mode)"
  "IsoFileIO::IsoFileIO(IsoArchiveIO *io, const IsoFile *infos, QObject *parent)"
[=[bool IsoArchiveIO::writeSector(const QByteArray &data, quint8 type, quint8 mode)
{
    const qint64 dataSize = data.size();
    const quint32 sectorCur = currentSector();
    QByteArray sectorData;

    Q_ASSERT(pos() % SECTOR_SIZE == 0);
    Q_ASSERT(dataSize <= SECTOR_SIZE_DATA);
    if (pos() % SECTOR_SIZE != 0 || dataSize < 0 || dataSize > SECTOR_SIZE_DATA || mode != 2) {
        return false;
    }

    sectorData = buildHeader(sectorCur, type, mode);
    sectorData.append(data);
    if (dataSize != SECTOR_SIZE_DATA) {
        sectorData.append(QByteArray(SECTOR_SIZE_DATA - dataSize, '\x00'));
    }

    // Reserve the complete raw-sector tail and generate the correct XA
    // Form-1 EDC/P/Q ECC (or Form-2 EDC when the submode requests Form 2).
    sectorData.append(QByteArray(SECTOR_SIZE_FOOTER, '\x00'));
    if (!ff7tk_psx_cd::updateSectorEdcEcc(sectorData)) {
        return false;
    }

    return write(sectorData) == SECTOR_SIZE;
}

]=])

_mr_replace_region("${_iso_cpp}"
  "bool IsoArchive::copySectors(IsoArchiveIO *out, qint64 sectorCount, ArchiveObserver *control, bool repair)"
  "bool IsoArchive::writeFile(QIODevice *in, quint32 sectorCount, ArchiveObserver *control)"
[=[bool IsoArchive::copySectors(IsoArchiveIO *out, qint64 sectorCount, ArchiveObserver *control, bool repair)
{
    if (sectorCount < 0) {
        qWarning() << "IsoArchive::copySectors sectorCount < 0" << sectorCount;
        setError(Archive::InvalidError);
        return false;
    }
    Q_ASSERT(out->pos() % SECTOR_SIZE == 0);
    Q_ASSERT(_io.pos() % SECTOR_SIZE == 0);

    for (qint64 i = 0; i < sectorCount; ++i) {
        if (control && control->observerWasCanceled()) {
            setError(Archive::AbortError);
            return false;
        }

        QByteArray data = _io.read(SECTOR_SIZE);
        if (data.size() != SECTOR_SIZE) {
            qWarning() << "IsoArchive::copySectors read error" << data.size() << SECTOR_SIZE;
            setError(Archive::ReadError, _io.errorString());
            return false;
        }

        const bool written = repair
                ? ff7tk_psx_cd::writeRelocatedSector(*out, data)
                : out->write(data) == SECTOR_SIZE;
        if (!written) {
            qWarning() << "IsoArchive::copySectors write error";
            setError(Archive::WriteError, out->errorString());
            return false;
        }

        if (control) {
            control->setObserverValue(int(out->currentSector()));
        }
    }

    return true;
}

]=])

# The volume descriptor size update bypasses writeIso upstream. Route these two
# payload writes through writeIso so the raw sector EDC/ECC is regenerated.
file(READ "${_iso_cpp}" _iso_text)
set(_old_volume_write1 "destinationIO->write((char*)&volume_space_size, 4);")
set(_old_volume_write2 "destinationIO->write((char*)&volume_space_size2, 4);")
string(FIND "${_iso_text}" "${_old_volume_write1}" _vol1)
string(FIND "${_iso_text}" "${_old_volume_write2}" _vol2)
if(_vol1 EQUAL -1 OR _vol2 EQUAL -1)
  message(FATAL_ERROR "ff7tk PSX fix: volume descriptor write markers not found")
endif()
string(REPLACE "${_old_volume_write1}"
               "destinationIO->writeIso((char*)&volume_space_size, 4);"
               _iso_text "${_iso_text}")
string(REPLACE "${_old_volume_write2}"
               "destinationIO->writeIso((char*)&volume_space_size2, 4);"
               _iso_text "${_iso_text}")
file(WRITE "${_iso_cpp}" "${_iso_text}")

# Sanity-check the transformed source before invoking ff7tk's CMake build.
file(READ "${_iso_cpp}" _iso_after)
foreach(_marker
    "#include \"psx-cdrom-edc-ecc.h\""
    "const qint64 startIsoPos = posIso();"
    "ff7tk_psx_cd::updateSectorEdcEcc(sectorData)"
    "ff7tk_psx_cd::writeRelocatedSector(*out, data)"
    "destinationIO->writeIso((char*)&volume_space_size, 4);")
  string(FIND "${_iso_after}" "${_marker}" _marker_pos)
  if(_marker_pos EQUAL -1)
    message(FATAL_ERROR "ff7tk PSX fix: transformed source missing marker: ${_marker}")
  endif()
endforeach()

vcpkg_cmake_configure(
  SOURCE_PATH ${SOURCE_PATH}
  OPTIONS
    "-DPACKAGE=OFF"
    "-DTESTS=OFF"
    "-DCMAKE_PROJECT_INCLUDE=${CMAKE_CURRENT_LIST_DIR}/qt.cmake"
)

vcpkg_cmake_install()
vcpkg_copy_pdbs()
vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/${PORT})
vcpkg_fixup_pkgconfig()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include" "${CURRENT_PACKAGES_DIR}/debug/lib/cmake" "${CURRENT_PACKAGES_DIR}/lib/cmake" "${CURRENT_PACKAGES_DIR}/debug/share" "${CURRENT_PACKAGES_DIR}/share/licenses")
file(INSTALL ${SOURCE_PATH}/COPYING.TXT DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)
