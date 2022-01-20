// **************************************************************************
// namespace async
// 02.07.2021
// async Utility classes
// **************************************************************************
// MIT License
// Copyright © 2021 Patrick Fial
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
// files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software
// is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

// **************************************************************************
// includes
// **************************************************************************

#ifndef ASYNC_HPP
#define ASYNC_HPP

#include <functional>
#include <QString>
#include <QList>
#include <QMap>
#include <QDebug>

// **************************************************************************
// namespace async
// **************************************************************************

namespace async
{
   using ErrCallback = std::function<void(QString)>;

   template<typename T>
   using VectorIteratorFunc = std::function<void(const T&, ErrCallback, int)>;

   template<typename T>
   using ListIteratorFunc = std::function<void(const T&, ErrCallback)>;

   template<typename K, typename V>
   using MapIteratorFunc = std::function<void(const K&, const V&, ErrCallback)>;

   // QList

   template <typename T>
   void seriesStep(typename QList<T>::const_iterator it, const QList<T>& container, ListIteratorFunc<T> f, ErrCallback cb)
   {
      if (it == container.end())
         return cb("");

      f(*it, [it, &container, f, cb](QString err) mutable
      {
         if (!err.isEmpty())
            return cb(err);

         it++;
         seriesStep(it, container, f, cb);
      });
   }

   template <typename T>
   void eachSeries(const QList<T>& container, ListIteratorFunc<T> f, ErrCallback cb)
   {
      seriesStep(container.begin(), container, f, cb);
   }

   // QMap

   template <typename K, typename V>
   void seriesStep(typename QMap<K,V>::const_iterator it, const QMap<K,V>& container, MapIteratorFunc<K, V> f, ErrCallback cb)
   {
      if (it == container.end())
         return cb("");

      f(it.key(), it.value(), [it, &container, f, cb](QString err) mutable
      {
         if (!err.isEmpty())
            return cb(err);

         it++;
         seriesStep(it, container, f, cb);
      });
   }

   template <typename K, typename V>
   void eachSeries(const QMap<K,V>& container, MapIteratorFunc<K, V> f, ErrCallback cb)
   {
      seriesStep(container.begin(), container, f, cb);
   }

   // std::vector

   template <typename T>
   void seriesStep(typename std::vector<T>::const_iterator it, const std::vector<T>& container, VectorIteratorFunc<T> f, ErrCallback cb)
   {
      if (it == container.end())
         return cb("");

      f(*it, [it, &container, f, cb](QString err) mutable
      {
         if (!err.isEmpty())
            return cb(err);

         it++;
         seriesStep(it, container, f, cb);
      }, it - container.begin());
   }

   template <typename T>
   void eachSeries(const std::vector<T>& container, VectorIteratorFunc<T> f, ErrCallback cb)
   {
      seriesStep(container.begin(), container, f, cb);
   }

} // namespace async

#endif // ASYNC_HPP
