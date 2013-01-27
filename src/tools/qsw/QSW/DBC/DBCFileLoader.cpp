#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DBCFileLoader.h"

DBCFileLoader::DBCFileLoader()
{
    data = NULL;
    fieldsOffset = NULL;
}

bool DBCFileLoader::Load(const char *filename, const char *fmt)
{

    quint32 header;
    if (data)
    {
        delete []data;
        data = NULL;
    }

    FILE *f = fopen(filename, "rb");
    if (!f)
        return false;

    if (fread(&header, 4, 1, f) != 1)                             // Number of records
        return false;

    EndianConvert(header);
    if (header != 0x43424457)
        return false;                                             //'WDBC'

    if (fread(&recordCount, 4, 1, f) != 1)                              // Number of records
        return false;

    EndianConvert(recordCount);

    if (fread(&fieldCount, 4, 1, f) != 1)                         // Number of fields
        return false;

    EndianConvert(fieldCount);

    if (fread(&recordSize, 4, 1, f) != 1)                         // Size of a record
        return false;

    EndianConvert(recordSize);

    if (fread(&stringSize, 4, 1, f) != 1)                         // String size
        return false;

    EndianConvert(stringSize);

    fieldsOffset = new quint32[fieldCount];
    fieldsOffset[0] = 0;
    for (quint32 i = 1; i < fieldCount; i++)
    {
        fieldsOffset[i] = fieldsOffset[i - 1];
        if (fmt[i - 1] == 'b' || fmt[i - 1] == 'X')         // byte fields
            fieldsOffset[i] += 1;
        else                                                // 4 byte fields (int32/float/strings)
            fieldsOffset[i] += 4;
    }

    data = new unsigned char[recordSize * recordCount + stringSize];
    stringTable = data + recordSize * recordCount;

    if (fread(data, recordSize * recordCount + stringSize, 1, f) != 1)
        return false;

    fclose(f);
    return true;
}

DBCFileLoader::~DBCFileLoader()
{
    if (data)
        delete []data;
    if (fieldsOffset)
        delete []fieldsOffset;
}

DBCFileLoader::Record DBCFileLoader::getRecord(size_t id)
{
    assert(data);
    return Record(*this, data + id * recordSize);
}

quint32 DBCFileLoader::GetFormatRecordSize(const char * format, qint32* index_pos)
{
    quint32 recordsize = 0;
    qint32 i = -1;
    for (quint32 x = 0; format[x]; ++x)
        switch (format[x])
        {
            case FT_FLOAT:
            case FT_INT:
                recordsize += 4;
                break;
            case FT_STRING:
                recordsize += sizeof(char*);
                break;
            case FT_SORT:
                i = x;
                break;
            case FT_IND:
                i = x;
                recordsize += 4;
                break;
            case FT_BYTE:
                recordsize += 1;
                break;
        }

    if (index_pos)
        *index_pos = i;

    return recordsize;
}

char* DBCFileLoader::AutoProduceData(const char* format, quint32& records, char**& indexTable)
{
    typedef char * ptr;
    if (strlen(format) != fieldCount)
        return NULL;

    //get struct size and index pos
    qint32 i;
    quint32 recordsize=GetFormatRecordSize(format, &i);

    if (i >= 0)
    {
        quint32 maxi = 0;
        //find max index
        for (quint32 y = 0; y < recordCount; y++)
        {
            quint32 ind=getRecord(y).getUInt(i);
            if (ind > maxi)
                maxi = ind;
        }

        ++maxi;
        records = maxi;
        indexTable = new ptr[maxi];
        memset(indexTable, 0, maxi*sizeof(ptr));
    }
    else
    {
        records = recordCount;
        indexTable = new ptr[recordCount];
    }

    char* dataTable = new char[recordCount * recordsize];

    quint32 offset=0;

    for (quint32 y = 0; y < recordCount; y++)
    {
        if (i >= 0)
        {
            indexTable[getRecord(y).getUInt(i)] = &dataTable[offset];
        }
        else
            indexTable[y] = &dataTable[offset];

        for (quint32 x = 0; x < fieldCount; x++)
        {
            switch (format[x])
            {
                case FT_FLOAT:
                    *((float*)(&dataTable[offset])) = getRecord(y).getFloat(x);
                    offset += 4;
                    break;
                case FT_IND:
                case FT_INT:
                    *((quint32*)(&dataTable[offset])) = getRecord(y).getUInt(x);
                    offset += 4;
                    break;
                case FT_BYTE:
                    *((quint8*)(&dataTable[offset])) = getRecord(y).getUInt8(x);
                    offset += 1;
                    break;
                case FT_STRING:
                    *((char**)(&dataTable[offset])) = NULL;   // will be replaces non-empty or "" strings in AutoProduceStrings
                    offset += sizeof(char*);
                    break;
            }
        }
    }

    return dataTable;
}

char* DBCFileLoader::AutoProduceStrings(const char* format, char* dataTable)
{
    if (strlen(format) != fieldCount)
        return NULL;

    char* stringPool = new char[stringSize];
    memcpy(stringPool, stringTable, stringSize);

    quint32 offset=0;

    for (quint32 y = 0; y < recordCount; y++)
    {
        for (quint32 x = 0; x < fieldCount; x++)
            switch (format[x])
            {
                case FT_FLOAT:
                case FT_IND:
                case FT_INT:
                    offset += 4;
                    break;
                case FT_BYTE:
                    offset += 1;
                    break;
                case FT_STRING:
                {
                    // fill only not filled entries
                    char** slot = (char**)(&dataTable[offset]);
                    if (!*slot || !**slot)
                    {
                        const char * st = getRecord(y).getString(x);
                        *slot = stringPool + (st - (const char*)stringTable);
                    }
                    offset += sizeof(char*);
                    break;
                }
                case FT_NA:
                case FT_NA_BYTE:
                case FT_SORT:
                    break;
                default:
                    assert(false && "unknown format character");
            }
    }

    return stringPool;
}
