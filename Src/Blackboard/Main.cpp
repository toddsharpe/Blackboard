#include "Blackboard.h"
#include <cstdint>

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
	/*
	 * uint8_t
	 * uint16_t
	 * uint32_t
	 * uint64_t
	 * int8_t
	 * int16_t
	 * int32_t
	 * int64_t
	 * float
	 * double
	 * str_t
	*/
	BlackboardStore store;
	BlackboardBuilder root(store, BlackboardOp);
	BlackboardView view = root.View();
	
	//u32
	{
		BlackboardBuilder sub = root.SubBuilder("u32");
		
		State<uint32_t> s;
		ASSERT(sub.Add("Test", s, 7));

		Input<uint32_t> i;
		ASSERT(sub.Get("Test", i));
		ASSERT(view[i] == 7);

		view[s] = 5;
		ASSERT(view[i] == 5);
	}

	//str
	{
		BlackboardBuilder sub = root.SubBuilder("str");
		
		Output<str_t> o;
		ASSERT(sub.Add("Test", o));
		view[o] = "test";

		Input<str_t> i;
		ASSERT(sub.Get("Test", i));
		ASSERT(strcmp(view[i].c_str(), "test") == 0);
	}

	//Generic interface
	{
		OutputToken o(TypeCode::u8);
		ASSERT(root.Add<uint8_t>("Component.Ch1", o));
		view.Set<uint8_t>(o, 5);

		InputToken i(TypeCode::u8);
		ASSERT(root.Get("Component.Ch1", i));
		ASSERT(view.Get<uint8_t>(i) == 5);
	}
}
