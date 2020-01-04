
int factorial(int n)
{
  if (n == 0)
    return 1;
  return n * factorial(n-1);
}

int fib(int n)
{
  if(n == 0) {
    return 0;
  }
  return fib(n-1) + fib(n-2);
}

int main()
{
  return factorial(5) + fib(4);
}