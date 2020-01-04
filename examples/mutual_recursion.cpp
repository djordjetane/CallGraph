int A(int n)
{
    if(n == 0)
        return 1;
    return B(n-1);
}

int B(int n)
{
    if(n == 0)
        return 0;
    return A(n-1);
}

int isEven(int n)
{
    return A(n);
}