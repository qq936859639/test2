#ifndef CAR_H
#define CAR_H

#include <QGraphicsObject>
#include <QBrush>

class Car : public QGraphicsObject
{
    Q_OBJECT
public:
    Car();
    QRectF boundingRect() const;

public Q_SLOTS:
    void accelerate();
    void decelerate();
    void turnLeft();
    void turnRight();

Q_SIGNALS:
    void crashed();

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
    void timerEvent(QTimerEvent *event);

private:
    QBrush color;
    qreal wheelsAngle; // used when applying rotation
    qreal speed; // delta movement along the body axis
};

#endif // CAR_H
