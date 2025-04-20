# Socow Vector
`vector` со small-object и copy-on-write оптимизациями.


## Методы `socow_vector`:
- Конструктор по умолчанию;
- Конструктор копирования;
- Конструктор перемещения;
- Оператор копирующего присваивания;
- Оператор перемещающего присваивания;
- `swap(socow_vector& other)` &mdash; поменять состояния текущего вектора и `other` местами;
- `size()` &mdash; размер вектора;
- `capacity()` &mdash; вместимость вектора;
- `empty()` &mdash; является ли вектор пустым;
- `operator[](std::size_t index)`; &mdash; обращение к элементу вектора;
- `front()`, `back()` &mdash; обращение к первому/последнему элементу вектора;
- `data()` &mdash; указатель на начало вектора;
- `begin()`, `end()` &mdash; итераторы;
- `push_back(...)` &mdash; вставить элемент в конец вектора (аргументом может быть lvalue или rvalue);
- `insert(const_iterator pos, ...)` &mdash; вставить элемент перед `pos`;
- `pop_back()` &mdash; удалить элемент из конца вектора;
- `erase(const_iterator pos)` &mdash; удалить элемент по итератору;
- `erase(const_iterator first, const_iterator last)` &mdash; удалить все элементы в диапазоне `[first, last)`;
- `clear()` &mdash; очистить вектор от всех элементов;
- `reserve(size_t new_capacity)` &mdash; установить вместимость вектора, если текущая меньше;
- `shrink_to_fit()` &mdash; сжать вместимость вектора до текущего размера.
