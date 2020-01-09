void G()
{
    
}
void F()
{
    D();
}
void E()
{

}
void D()
{
    G();
}
void C()
{

}
void B()
{
    C();
    D();
}
void A()
{
    B();
    E();
    F();
}

int main()
{
    A();
}