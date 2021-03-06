#ifndef SQLSTATEMENTHELPER_H
#define SQLSTATEMENTHELPER_H


#include "postgresqldatabaseutil.h"

namespace Ilwis {

class SPFeatureI;

namespace Postgresql {

class SqlStatementHelper
{
public:
    SqlStatementHelper(const PostgresqlDatabaseUtil &pgUtil);
    ~SqlStatementHelper();

    void addCreateTempTableStmt(const QString &tmpTableName);
    QString createWhereComparingPrimaryKeys(const QString &first, const QString &second) const;
    void addInsertChangedDataToTempTableStmt(const QString &tmpTable, const Table *table);
    void addUpdateStmt(const QString &tmpTable, const Table *table);
    void addInsertStmt(const QString &tmpTable, const Table *table);
    void addDeleteStmt(const QString &tmpTable, const Table *table);

    QString columnNamesCommaSeparated(const Table *table) const;
    QString columnValuesCommaSeparated(const SPFeatureI feature) const;
    QString createInsertValueString(QVariant value, const ColumnDefinition &coldef) const;
    QString trimAndRemoveLastCharacter(const QString &string) const;

    QString sql();

private:
    PostgresqlDatabaseUtil _pgUtil;
    QList<QString> _tmpTables;
    QString _sqlBuilder;
};

}
}

#endif // SQLSTATEMENTHELPER_H
