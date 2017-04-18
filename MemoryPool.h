#include<iostream>
using namespace std;
template<class T>
struct PoolNode
{
	void* memory; //指向申请内存块的头
	size_t n;   //需要申请的对象的个数
	PoolNode<T>* next;   //指向下一个申请内存块的指针

	PoolNode(size_t nobjs)
	{
		n = nobjs;
		//这里使用operator new，一是不用调用构造函数了，
		//二是它里面有处理异常的操作，如果申请失败的话，
		//就直接抛出异常
		memory = ::operator new(MemoryPool<T>::GetObjSize()*n);
		next = NULL;
	}

	~PoolNode()
	{
		::operator delete(memory);
		memory = NULL;

	}


};

template<class T>
class MemoryPool
{
	typedef PoolNode<T> Node;
public:
	//对象的构造
	MemoryPool(size_t initnobjs = 8,size_t initmax = 1024)
		:_initnobjs(initnobjs)
		,_initmax(initmax)
		//,_head(NULL)
		//,_tail(NULL)
		,_lastdelete(NULL)
	{
		_head = _tail = new Node(initnobjs);
		_usecount = 0;
	}

	~MemoryPool()
	{
		Node* cur =_head;
		while(cur->next)
		{
			Node* ret = cur->next;
			delete cur;
			ret = cur;
		}
		
	}

	void* Allocate()
	{
		//如果有对象的释放的话，先去释放回来的那些内存块中找
		if(_lastdelete)
		{
			void* obj = _lastdelete;
			_lastdelete = *(T**)(_lastdelete);
			return obj;
		}
		//否则就要去申请节点中去找
		if(_usecount >= _tail->n)
		{
			_Allocate();
		}
		void* obj = (char*)(_tail->memory)+_usecount*GetObjSize();
		_usecount++;
		return obj;
	}

	void Deallocate(T* ptr)
	{
		//其实就是将还回来的内存块头插到_lastdelete维护的链表中
		*(T**) ptr = _lastdelete;
		_lastdelete = (T*)ptr;

	}

	//利用自己写的内存池构造和析构
	template<class V>
	T* New(const V& v)
	{
		void* obj = Allocate();
		return new(obj)T(v);
	}
	 void Delete(T* ptr)
	{
		if(ptr)
		{
			ptr->~T();
			Deallocate(ptr);
		}
	}
protected:
	void _Allocate()
	{
		size_t n = _tail->n*2;
		if(n>_initmax)
		{
			n = _initmax;
		}
		Node* node = new Node(n);
		_tail->next = node;
		_tail = node;
		_usecount = 0;
		//return node;
	}
public:
	inline static size_t GetObjSize()
	{
		return sizeof(T)>sizeof(T*)?sizeof(T):sizeof(T*);
	}
private:
	size_t _initnobjs;
	size_t _initmax;

	Node* _head;    //对象申请内存块的头
	Node* _tail;    //对象申请内存块的尾

	size_t _usecount; //挂接的内存块的个数

	T* _lastdelete;   //维护还回来内存块的链表


};

void TestPool()
{
	MemoryPool<int> mpool;
	void*mp1 = mpool.Allocate();
	void*mp2 = mpool.Allocate();

	MemoryPool<string> mpool1;
	string* mp3 = mpool1.New("上海");
	mpool1.Delete(mp3);
	


}