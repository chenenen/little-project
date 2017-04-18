#include<iostream>
using namespace std;
template<class T>
struct PoolNode
{
	void* memory; //ָ�������ڴ���ͷ
	size_t n;   //��Ҫ����Ķ���ĸ���
	PoolNode<T>* next;   //ָ����һ�������ڴ���ָ��

	PoolNode(size_t nobjs)
	{
		n = nobjs;
		//����ʹ��operator new��һ�ǲ��õ��ù��캯���ˣ�
		//�����������д����쳣�Ĳ������������ʧ�ܵĻ���
		//��ֱ���׳��쳣
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
	//����Ĺ���
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
		//����ж�����ͷŵĻ�����ȥ�ͷŻ�������Щ�ڴ������
		if(_lastdelete)
		{
			void* obj = _lastdelete;
			_lastdelete = *(T**)(_lastdelete);
			return obj;
		}
		//�����Ҫȥ����ڵ���ȥ��
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
		//��ʵ���ǽ����������ڴ��ͷ�嵽_lastdeleteά����������
		*(T**) ptr = _lastdelete;
		_lastdelete = (T*)ptr;

	}

	//�����Լ�д���ڴ�ع��������
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

	Node* _head;    //���������ڴ���ͷ
	Node* _tail;    //���������ڴ���β

	size_t _usecount; //�ҽӵ��ڴ��ĸ���

	T* _lastdelete;   //ά���������ڴ�������


};

void TestPool()
{
	MemoryPool<int> mpool;
	void*mp1 = mpool.Allocate();
	void*mp2 = mpool.Allocate();

	MemoryPool<string> mpool1;
	string* mp3 = mpool1.New("�Ϻ�");
	mpool1.Delete(mp3);
	


}