#include "Blackboard.h"
#include <cstdint>

int main(int argc, char** argv)
{
	BlackboardStore store;
	BlackboardBuilder root(store);

	State<uint32_t> s;
	_ASSERT(root.Add("Test", s, 7));

	Input<uint32_t> i;
	_ASSERT(root.Get("Test", i));

	BlackboardBuilder sub = root.SubBuilder("Component");
	State<uint8_t> s1;
	_ASSERT(sub.Add("T1", s1));

	Input<uint8_t> i2;
	_ASSERT(sub.Get("T1", i2));

	Input<uint8_t> i3;
	_ASSERT(root.Get("Component.T1", i3));

	BlackboardView view = root.View();

	const uint32_t v = view.Get(i);
	_ASSERT(v == 7);
	view.Set(s, 5);
	const uint32_t v2 = view.Get(i);
	_ASSERT(v2 == 5);
}
