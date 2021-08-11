#include "CircularBuffer.h"

using namespace std;


int main(int argc, char *argv[])
{
    Q_UNUSED(argc)
    Q_UNUSED(argv)

    CircularBuffer<int> a(20);
    a.putLast(0);
    a.putLast(1);
    a.putLast(2);
    a.putLast(3);
    a.putLast(4);
    a.putLast(5);
    a.putLast(6);
    a.putLast(7);
    a.putLast(8);

    QVector<int> indexes = {0, 7, 0, 1, 2, 0, 1, 2, 0, 1, 2};
    auto selective = a.subBuffer(indexes);
    for (auto& i : selective) {
        qDebug() << i;
        i = 1;
    }

    return 1;
}


