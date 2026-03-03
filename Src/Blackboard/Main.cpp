#include "Blackboard.h"
#include <cstdint>

#define ASSERT _ASSERT

void BlackboardOp(const BlackboardBuilder::Op op, const char* name, const TypeCode type)
{
	printf("Op: %d, Name: %s, Type: %s(%d)\n",
		static_cast<uint32_t>(op),
		name,
		GetTypeName(type),
		static_cast<uint32_t>(type));
}

int main(int argc, char** argv)
{
	BlackboardStore store;
	BlackboardBuilder root(store, BlackboardOp);
	BlackboardView view = root.View();

	{
		State<uint32_t> s;
		ASSERT(root.Add("Test", s, 7));

		Input<uint32_t> i;
		ASSERT(root.Get("Test", i));

		ASSERT(view.Get(i) == 7);
		view.Set(s, 5);
		ASSERT(view.Get(i) == 5);
	}

	BlackboardBuilder sub = root.SubBuilder("Component");
	{
		State<uint8_t> s;
		ASSERT(sub.Add("T1", s));
		view.Set(s, 3);

		Input<uint8_t> i;
		ASSERT(sub.Get("T1", i));

		ASSERT(view.Get(i) == 3);
		view.Set(s, 5);
		ASSERT(view.Get(i) == 5);
	}

	{
		Input<uint8_t> i;
		ASSERT(root.Get("Component.T1", i));
		ASSERT(view.Get(i) == 5);
		ASSERT(view[i] == 5);
	}

	{
		Output<float> f;
		ASSERT(sub.Add("Pwr", f));
		view.Set(f, 3.3);
	}

	{
		Output<str_t> c;
		root.Add("Version", c, "Test");

		Input<str_t> c2;
		root.Get("Version", c2);
		ASSERT(strcmp(view.Get(c2), "Test") == 0);
	}
}
