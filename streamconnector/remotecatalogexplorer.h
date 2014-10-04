#ifndef REMOTECATALOGEXPLORER_H
#define REMOTECATALOGEXPLORER_H

namespace Ilwis {
namespace Stream {
class RemoteCatalogExplorer : public CatalogExplorer
{
public:
    RemoteCatalogExplorer(const Ilwis::Resource &resource,const IOOptions& options=IOOptions());

    std::vector<Resource> loadItems();
    bool canUse(const Resource& resource) const;
    QString provider() const;

    static Ilwis::CatalogExplorer *create(const Ilwis::Resource &resource, const Ilwis::IOOptions &options);
};
}
}

#endif // REMOTECATALOGEXPLORER_H