
#include <QSqlError>
#include <QSqlDatabase>

#include "kernel.h"
#include "ilwisdata.h"
#include "ilwistypes.h"
#include "resource.h"
#include "domain.h"
#include "geometries.h"
#include "numericdomain.h"
#include "textdomain.h"
#include "datadefinition.h"
#include "columndefinition.h"
#include "attributedefinition.h"
#include "coverage.h"
#include "featurecoverage.h"
#include "feature.h"
#include "table.h"

#include "postgresqltableloader.h"
#include "postgresqldatabaseutil.h"

using namespace Ilwis;
using namespace Postgresql;

PostgresqlTableLoader::PostgresqlTableLoader(const Resource &resource, const IOOptions &options): _resource(resource), _options(options)
{
}

PostgresqlTableLoader::~PostgresqlTableLoader()
{
}

bool PostgresqlTableLoader::loadMetadata(Table *table) const
{
    //qDebug() << "PostgresqlTableLoader::loadMetadata()";

    PostgresqlDatabaseUtil pgUtil(_resource, _options);
    QString rawTablename(pgUtil.tablenameFromResource());

    QString sqlBuilder;
    sqlBuilder.append("SELECT ");
    sqlBuilder.append(" column_name,udt_name ");
    sqlBuilder.append(" FROM ");
    sqlBuilder.append(" information_schema.columns ");
    sqlBuilder.append(" WHERE ");
    sqlBuilder.append(" table_name='").append(rawTablename).append("';");
    //qDebug() << "SQL: " << sqlBuilder;

    QSqlQuery columnTypesQuery = pgUtil.doQuery(sqlBuilder, "tableloader");

    QList<QString> primaryKeys;
    pgUtil.getPrimaryKeys(primaryKeys);

    while (columnTypesQuery.next()) {
        QString columnName = columnTypesQuery.value(0).toString();
        if ( !createColumnDefinition(table, columnTypesQuery, primaryKeys)) {
            if ( !columnTypesQuery.isValid()) {
                WARN("no data record selected.");
            } else {
                DEBUG2("Ignore column '%1' in table '%2'", columnName, rawTablename);
            }
        }
    }
    return table->isValid();
}

QString PostgresqlTableLoader::select(QString columns) const
{
    QString sqlBuilder;
    sqlBuilder.append("SELECT ");
    sqlBuilder.append(columns);
    sqlBuilder.append(" FROM ");

    PostgresqlDatabaseUtil pgUtil(_resource, _options);
    sqlBuilder.append(pgUtil.qTableFromTableResource());
    //qDebug() << "SQL: " << sqlBuilder;
    return sqlBuilder;
}

bool PostgresqlTableLoader::loadData(Table *table) const
{
    //qDebug() << "PostgresqlTableLoader::loadData()";

    QString allNonGeometryColumns;
    for (int i = 0; i < table->columnCount(); i++) {
        ColumnDefinition& coldef = table->columndefinitionRef(i);
        allNonGeometryColumns.append(" ").append(coldef.name()).append(" ,");
    }
    allNonGeometryColumns = allNonGeometryColumns.left(allNonGeometryColumns.length() - 1);

    PostgresqlDatabaseUtil pgUtil(_resource, _options);
    QSqlQuery query = pgUtil.doQuery(select(allNonGeometryColumns), "tableloader.loadData");

    quint64 count = 0;
    while (query.next()) {
        std::vector<QVariant> record(table->columnCount());
        for (int i = 0; i < table->columnCount(); i++) {
            ColumnDefinition& coldef = table->columndefinitionRef(i);
            DataDefinition& datadef = coldef.datadef();
            if( !datadef.domain().isValid()) {
                WARN2(ERR_NO_INITIALIZED_2, "domain", coldef.name());
                record[i] = QVariant(); // empty
                continue;
            }
            record[i] = query.value(coldef.name());
        }
        table->record(count++, record);
    }
    return true;
}

bool PostgresqlTableLoader::createColumnDefinition(Table *table, const QSqlQuery &query, QList<QString> &primaryKeys) const
{
    if ( !query.isValid()) {
        WARN("Can't create columndefinition for an invalid data record.");
        return false;
    }

    QString columnName = query.value(0).toString();
    QString udtName = query.value(1).toString();

    IDomain domain;
    if (udtName == "varchar" || udtName == "text" || udtName == "char") {
        domain.prepare("code=domain:text",itTEXTDOMAIN);
    } else if (udtName.startsWith("timestamp")) {
        // TODO timestamp vs. timestamptz?
        ITimeDomain tdomain;
        tdomain.prepare();
        tdomain->range(new TimeInterval(itDATE));
        domain = tdomain;
    } else if (udtName.startsWith("float")) {
        INumericDomain ndomain;
        ndomain.prepare("value");
        domain = ndomain;
    } else if (udtName.startsWith("int")) {
        INumericDomain ndomain;
        ndomain.prepare("integer");
        domain = ndomain;
    } else if (udtName == "bool") {
        domain.prepare("boolean", itBOOL);
    } else if (udtName == "geometry") {
        return true; // handled automatically
    } else {
        MESSAGE1("No domain handle for db type '%1')", udtName);
        return false;
    }

    bool readonly = primaryKeys.contains(columnName);
    table->addColumn(columnName, domain, readonly);
    return true;
}
