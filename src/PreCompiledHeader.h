/// C files here

#if defined __cplusplus

/// C++ files here

#include <memory>
#include <optional>
#include <functional>
#include <algorithm>
#include <utility>
#include <array>
#include <vector>

#include <QObject>
#include <QString>
#include <QVector>
#include <QDebug>
#include <QHash>
#include <QDateTime>
#include <QTimer>
#include <QQueue>
#include <QMutex>
#include <QMutexLocker>

#include <Eigen/Dense>

template <class T>
using sp = std::shared_ptr<T>;

template <class T>
using up = std::unique_ptr<T>;

template <class T>
using option = std::optional<T>;

#endif
