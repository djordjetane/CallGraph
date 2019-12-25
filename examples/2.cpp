
struct hello
{
    virtual void VirtualFunc()
    {
        int g;
    }

    template<typename T>
    void TempalteMember(T t)
    {

    }
};

struct world : hello
{
    void VirtualFunc() override
    {

    };
};

template<typename T>
void TemplateFree(const T& t)
{

}

void g()
{
    hello h;
    h.VirtualFunc();
    hello *h2 = new world;
    h2->VirtualFunc();
    h2->TempalteMember(h);
    h2->TempalteMember(std::move(h));
    TemplateFree(h2);
}
