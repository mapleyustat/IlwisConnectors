#include "kernel.h"
#include "ilwisdata.h"
#include "symboltable.h"
#include "commandhandler.h"
#include "operationExpression.h"
#include "operationmetadata.h"
#include "operation.h"
#include "catalog.h"
#include "mastercatalog.h"

#include "postgresqlconnection.h"

using namespace Ilwis;
using namespace Postgresql;

REGISTER_OPERATION(PostgresqlConnection)

PostgresqlConnection::PostgresqlConnection(quint64 metaid, const Ilwis::OperationExpression &expression) : OperationImplementation(metaid, expression)
{

}

bool PostgresqlConnection::execute(ExecutionContext *ctx, SymbolTable &symTable)
{
    if (_prepState == sNOTPREPARED)
        if((_prepState = prepare(ctx,symTable)) != sPREPARED)
            return false;

    QString expression = "postgresql://"+ _host;
    if ( _port != 0)
        expression += ":" + QString::number(_port);
    if ( _database != "") {
        expression += "/" + _database;
    }
    if ( _schema != "") {
        expression += "/" + _schema;
    }
    expression += QString("/%1/%2").arg(_database).arg(_schema);
    if (_table != "") {
        expression += QString("/%1").arg(_table);
    }

    if ( _username != "" && _password != ""){
        expression += QString("?username=%1&password=%2").arg(_username).arg(_password);
    }

    QVariant value;
    value.setValue<QString>(expression);
    ctx->setOutput(symTable, value, "connectionstring", itSTRING, Resource());

    return true;
}

Ilwis::OperationImplementation *PostgresqlConnection::create(quint64 metaid, const Ilwis::OperationExpression &expr)
{
    return  new PostgresqlConnection(metaid, expr);
}

Ilwis::OperationImplementation::State PostgresqlConnection::prepare(ExecutionContext *ctx, const SymbolTable &)
{
    _host = _expression.input<QString>(0);
    _port = _expression.input<quint32>(1);
    _database = _expression.input<QString>(2);
    _schema = _expression.input<QString>(3);
    _table = _expression.input<QString>(4);
    _username = _expression.input<QString>(5);
    _password = _expression.input<QString>(6);
    if ( _host == "")
        return sPREPAREFAILED;
    if ( _schema == "")
        return sPREPAREFAILED;

    return sPREPARED;
}

quint64 PostgresqlConnection::createMetadata()
{
    OperationResource operation({"ilwis://operations/postgresqlcatalog"});
    operation.setLongName("PostgreSQL Catalog");
    operation.setSyntax("wfscatalog(host[,port],path[,username, password])");
    operation.setDescription(TR("creates a url to access the catalog of a wfs server"));
    operation.setInParameterCount({3,4,5,6,7});
    operation.addInParameter(0,itSTRING , TR("host address"),TR("identifies the host that is running the posgresql database"));
    operation.addInParameter(1,itPOSITIVEINTEGER , TR("port number"),TR("port used on the remote server"));
    operation.addInParameter(2,itSTRING , TR("database"),TR("The database name on the server"));
    operation.addInParameter(3,itSTRING , TR("schema"),TR("The database schema in the database"));
    operation.addInParameter(4,itSTRING , TR("table"),TR("A database table in the database"));
    operation.addInParameter(5,itSTRING , TR("username"),TR("username for authentication on the remote server"));
    operation.addInParameter(6,itSTRING , TR("password"),TR("password for authentication on the remote server"));
    operation.setOutParameterCount({1});
    operation.addOutParameter(0,itSTRING, TR("connection url"),TR("string that is sufficient to access the remote catalog"));
    operation.setKeywords("database,postgresql");

    mastercatalog()->addItems({operation});
    return operation.id();
}

