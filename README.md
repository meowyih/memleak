# Detect memory leak in C++ using macro and operator overriding

## Solution 1

Using macro to replace 'new int' into 'new(__FILE__,__LINE) int' and override the 'new(size_t, const char*, int)'

``` c++
    void* operator new(size_t size, const char* file, int line) {...};
    void* operator new[](size_t size, const char* file, int line) {...};

    void operator delete(void* p) {...};
    void operator delete[](void* p) {...};

    #define new new(__FILE__, __LINE__)
```

### Problem:

* It does not work on Class object creation.

``` c++
    MyClass* cls = new MyClass(); // this one won't work
```

* It does not work on placement new.

``` c++
    int* x = new(std::nothorw) int; // this one won't work
```
    
## Solution 2 

Replace 'new int' into 'Memleak(__FILE__,__LINE__) << new' and overload operator << in Memleak class.

``` c++
class Memleak
{
public:
    Memleak(const char* file, int line) : file_(file), line_(line) {}

    template <class T>
    T * operator << (T* t) const
    {
        // allocate an object t at line_ in file_
        return t;
    }

private:
    const char* file_;
    int line_;
};

#define new Memleak(__FILE__,__LINE__) << new

void operator delete(void* p) {...}
void operator delete[](void* p) {...}
```

### Problem

It does not work when delete an class object, like this:

``` c++
    MyClass* cls = new MyClass(); // this one works
    delete cls; // this one calls class' static delete member instead of global
```

## Solution 3

Similar to solution 2, but also replace 'delete x' into 'Memleak(__FILE__,__LINE__) >>' and overload operator >> in Memleak class.

``` c++
class Memleak
{
public:
    Memleak(const char* file, int line) : file_(file), line_(line) {}

    template <class T>
    T * operator << (T* t) const
    {
        MemleakRecorder::instance().alloc(file_, line_, t);
        return t;
    }

    template <class T>
    T * operator >> (T* t) const
    {
        MemleakRecorder::instance().release(t);
        delete t;
        return t;
    }

private:
    const char* file_;
    int line_;
};

#define new Memleak(__FILE__,__LINE__) << new
#define delete Memleak(__FILE__,__LINE__) >>
```

### Problem

The keyword 'delete' is not only for deleting dynamic object usage. For example:

``` c++
    class MyClass
    {
    public:
        // 'delete' will be replaced by macro,
        // this kind of syntax happens a lot in STL,
        // it almost not possible to avoid.
        MyClass(MyClass const&) = delete;
    };
```

## Solution 4

Override both global and in-class new/delete operator

``` c++
void* operator new(size_t size) {...}
void* operator new[](size_t size) {...}
void operator delete(void* p) {...}
void operator delete[](void* p) {...}

class MyClass
{
...
#ifdef DEBUG_MEMORY
public:
    static void* operator new(size_t size) {...}
    static void* operator new[](size_t size) {...}
    static void operator delete(void* p) {...}
    static void operator delete[](void* p) {...}
#endif
};
```

### Problem:

Debug information such as __FILE__ and __LINE__ only provides information that not very useful. We know which class has memory leak but does not know where it had been created.

## Conclusion

Personally I uses solution 4 for the project that does not have it own memory pool at beginning. Although debug information is not enough, but better than nothing.
