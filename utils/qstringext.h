#ifndef QSTRINGEXT_H
#define QSTRINGEXT_H
#include <QString>

#define ext(x) QStringExt(x)

class QStringExt : public QString
{
    public:
        QStringExt():QString(){}
        QStringExt(QString string):QString(string){}
        QStringExt(QByteArray array):QString(array){}
        QStringExt(char *&charArray):QString(charArray){}
        QStringExt(const char* charArray):QString(charArray){}
        QStringExt(const char charArray):QString(charArray){}

        QStringExt afterLast(QChar word);
        QStringExt afterLast(QString word);

        QStringExt beforeLast(QChar word);
        QStringExt beforeLast(QString word);

        QStringExt afterFirst(QString word);
        QStringExt afterFirst(QChar word);

        QStringExt beforeFirst(QString word);
        QStringExt beforeFirst(QChar word);

        static QStringExt fromStdWString(const std::wstring &s);

        using QString::operator+=;
        using QString::operator=;
};

#endif // QSTRINGEXT_H
