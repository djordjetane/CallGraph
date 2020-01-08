template<typename Func, typename ...Args>
auto ForwardIt(Func func, Args&& ...args)
{
	return func(args...);
}

template<typename T>
struct vector {};

template<typename Index>
vector<int> GetAllIndexes(Index from, Index to)
{
    vector<int> result;

    return result;
}

struct base
{
    void BaseNormal()
    {
		ForwardIt(GetAllIndexes<int>, 10, 20);
    }
};
int f() { return 0; }
struct derived : public base
{
    void BaseNormal(base* base) override
    {
		ForwardIt(f);
    }
};

void g() {}

int main()
{
	base *b = new derived;
	b->BaseVirtual(b);
	b->BaseNormal();
	derived d;
	d.BaseNormal(&d);
	return 0;
}





