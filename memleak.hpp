#ifndef MEMLEAK_HEADER
#define MEMLEAK_HEADER

#include <iostream>
#include <cstddef>
#include <cstdlib>
#include <map>
#include <string>

#define MALLOC malloc
#define FREE   free

// https://groups.google.com/d/msg/comp.lang.c++.moderated/cIk4LjvBxIk/6Z7HHXoQ-YUJ
// simple, non-sorted linked list that stores file name, line number and pointer address
class MemleakDataList
{
private:
    class Data
    {
    public:
        void* ptr_;
        const char* fname_;
        int line_;

        Data* next_;
    };

public:
    MemleakDataList() 
    {
        // create empty list only contains begin_ and end_
        end_ = (Data*)MALLOC(sizeof(Data));
        end_->next_ = NULL;
        begin_ = (Data*)MALLOC( sizeof( Data ));
        begin_->next_ = end_;
        
        size_ = 0;
    }

    ~MemleakDataList() 
    {
        // release all Data except begin_ and end_
        Data* anchor = begin_->next_;
        while (anchor != end_ )
        {
            Data* next = anchor->next_;
            FREE( anchor );
            anchor = next;
        }

        FREE(begin_);
        FREE(end_);
    }

    void add(const char* fname, int line, void* ptr)
    {
        // create a data
        Data* data = (Data*)MALLOC(sizeof(Data));
        data->fname_ = fname;
        data->line_ = line;
        data->ptr_ = ptr;

        // insert to head_->next_
        data->next_ = begin_->next_;
        begin_->next_ = data;

        size_++;
    }

    void remove(void* ptr)
    {
        Data* before = begin_;
        Data* anchor = begin_->next_;

        while (anchor != end_)
        {
            if (anchor->ptr_ == ptr)
            {
                before->next_ = anchor->next_;
                FREE(anchor);
                size_--;
                return;
            }

            before = anchor;
            anchor = anchor->next_;
        }
    }

    size_t size() { return size_; }

    void dump(std::ostream& os)
    {
        Data* anchor = begin_->next_;

        while (anchor != end_)
        {
            os << "addr:" << anchor->ptr_ << " file:" << anchor->fname_ << " line:" << anchor->line_ << std::endl;
            anchor = anchor->next_;
        }
    }

private:
    Data* begin_;
    Data* end_;
    size_t size_;
};

class MemleakRecorder
{
public:
    // singleton
    static MemleakRecorder& instance()
    {
        static MemleakRecorder instance;
        return instance;
    }

    void alloc( const char* file, int line, void* ptr )
    {
        datalist_.add(file, line, ptr);
    }

    void release(void* ptr)
    {
        datalist_.remove(ptr);
    }

private:
    MemleakRecorder() {}
    ~MemleakRecorder()
    {
        if (datalist_.size() != 0)
        {
            std::cerr << "memory leak detected, count:" << datalist_.size() << std::endl;

            datalist_.dump(std::cerr);
        }
    }

private:
    // should implement my own data structor to avoid new/delete in new/delete
    MemleakDataList datalist_;
};

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

// #define new Memleak(__FILE__,__LINE__) << new
// #define delete Memleak(__FILE__,__LINE__) >>

void* operator new(size_t size);

void* operator new[](size_t size);

void operator delete(void* p);

void operator delete[](void* p);

#if 0
void* operator new(size_t size, const char* file, int line)
{
    void* memory = MALLOC(size);

    MemleakRecorder::instance().alloc(file, line, memory);

    return memory;
}

void* operator new[](size_t size, const char* file, int line)
{
    void* memory = MALLOC(size);

    MemleakRecorder::instance().alloc(file, line, memory);

    return memory;
}

void operator delete(void* p)
{
    MemleakRecorder::instance().release(p);
    FREE(p);
}

void operator delete[](void* p)
{
    MemleakRecorder::instance().release(p);
    FREE(p);
}

// overwrite 'placement new'
#define new new(__FILE__, __LINE__)
#endif

#endif
