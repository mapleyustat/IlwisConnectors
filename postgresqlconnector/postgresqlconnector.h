#ifndef POSTGRESQLCONNECTOR_H
#define POSTGRESQLCONNECTOR_H

namespace Ilwis {
namespace Postgresql {

class PostgresqlConnector : public IlwisObjectConnector
{
public:
    PostgresqlConnector(const Ilwis::Resource &resource, bool load=true,const IOOptions& options=IOOptions());
    virtual ~PostgresqlConnector();

    bool loadMetaData(IlwisObject* data,const IOOptions&);

    QString provider() const;
private:
    QString _host;
    QString _database;
    QString _user;
    QString _password;
};
}
}

#endif // POSTGRESQLCONNECTOR_H
