#include "qstringext.h"

#include <QDebug>
#include <QString>

QStringExt QStringExt::afterLast(QChar word)
{
    int index = lastIndexOf(word)+1;
    return right(size()-index);
}

QStringExt QStringExt::afterLast(QString word)
{
    int index = lastIndexOf(word);
    return right(size()-index-word.length());
}

QStringExt QStringExt::beforeLast(QChar word)
{
    int index = lastIndexOf(word);
    return left(index);
}

QStringExt QStringExt::beforeLast(QString word)
{
    int index = lastIndexOf(word);
    return left(index);
}

QStringExt QStringExt::afterFirst(QString word)
{
    int index = indexOf(word);
    index = length()-index-word.length();
    return right(index);
}

QStringExt QStringExt::afterFirst(QChar word)
{
    int index = indexOf(word)+1;
    index = length()-index;
    return right(index);
}

QStringExt QStringExt::beforeFirst(QString word)
{
    int index = indexOf(word);
    return left(index);
}

QStringExt QStringExt::beforeFirst(QChar word)
{
    int index = indexOf(word);
    return left(index);
}

QStringExt QStringExt::fromStdWString(const std::wstring &s)
{
    return QString::fromStdWString(s);
}

