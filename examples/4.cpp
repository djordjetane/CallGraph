void ABCD_1()
{}
void AbCC_2()
{ ABCD_1(); }

void someAwful_name_staznamAbbCC()
{ AbCC_2(); }
void ABC()
{}

int main()
{

	someAwful_name_staznamAbbCC();
	ABC();

	return 0;
}

