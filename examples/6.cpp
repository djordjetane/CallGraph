template<typename Func, typename ...Args>
void ForwardIt(Func func, Args&& ...args)
{
	func(args...);
}

void g(int, double, char)
{}
void f()
{}

int main()
{
	ForwardIt(g, 1, 3.4, 'a');
	ForwardIt(f);
	return 0;
}