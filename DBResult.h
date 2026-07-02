#ifndef DBRESULT_H
#define DBRESULT_H

#include <string>
#include <vector>
#include <stdlib.h>

class DBResult
{
public:
    std::vector<std::string> columnNames;
    std::vector<int> columnTypes;
    std::vector< std::vector<std::string> > rows;

    void Clear()
    {
        columnNames.clear();
        columnTypes.clear();
        rows.clear();
    }

    void AddColumn(const std::string& name, int type)
    {
        columnNames.push_back(name);
        columnTypes.push_back(type);
    }

    void AddRow(const std::vector<std::string>& row)
    {
        rows.push_back(row);
    }

    int RowCount() const
    {
        return rows.size();
    }

    int ColumnCount() const
    {
        return columnNames.size();
    }

    std::string ColumnName(int col) const
    {
        if (col < 0 || col >= (int)columnNames.size())
            return "";
        return columnNames[col];
    }

    int ColumnType(int col) const
    {
        if (col < 0 || col >= (int)columnTypes.size())
            return 0;
        return columnTypes[col];
    }

    std::string GetString(int row, int col) const
    {
        if (row < 0 || row >= (int)rows.size())
            return "";

        if (col < 0 || col >= (int)rows[row].size())
            return "";

        return rows[row][col];
    }

    int GetInt(int row, int col) const
    {
        return atoi(GetString(row, col).c_str());
    }

    bool GetBool(int row, int col) const
    {
        std::string v = GetString(row, col);

        return (v == "t" ||
                v == "true" ||
                v == "TRUE" ||
                v == "1" ||
                v == "yes" ||
                v == "YES");
    }
};

#endif
