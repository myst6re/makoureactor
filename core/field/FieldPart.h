#ifndef FIELDPART_H
#define FIELDPART_H

#include <QtCore>

class Field;

class FieldPart
{
public:
	FieldPart(Field *field);
	virtual ~FieldPart();

	virtual bool open()=0;
	virtual bool open(const QByteArray &data)=0;
	virtual QByteArray save() const=0;
	virtual bool canSave() const;
	virtual void close();
	virtual void clear()=0;

	bool isOpen() const;
	virtual bool isModified() const;
	void setModified(bool modified);
protected:
	void setOpen(bool open);
	Field *field() const;
private:
	bool modified, opened;
	Field *_field;
};

#endif // FIELDPART_H
