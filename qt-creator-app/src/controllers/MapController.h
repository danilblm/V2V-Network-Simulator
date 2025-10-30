#ifndef MAPCONTROLLER_H
#define MAPCONTROLLER_H

#include <QObject>
#include <QVariantList>
#include <QGeoCoordinate>

class MapController : public QObject {
    Q_OBJECT
public:
    explicit MapController(QObject *parent = nullptr);
    Q_INVOKABLE void loadOSMData(const QString &path);

signals:
    void roadReady(const QVariantList &roads);
};

#endif // MAPCONTROLLER_H
