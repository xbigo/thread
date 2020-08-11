#include <catch2/catch.hpp>
#include <ape/thread/threadpool.hpp>

int foo(int x) { return x + 42; }
void test(){

	using namespace ape::thread;
	auto tpool = thread_pool::create();
	auto f = tpool->async(&foo, 4);
	CHECK(f.get() == 42);
	tpool->stop();
}

TEST_CASE("ape::thread::thread_pool", "[ape.thread.pool]"){
	using namespace ape::thread;
	using namespace std::chrono_literals;

	SECTION("Check global thread pool"){
		set_default_thread_pool_concurrency(2);

		CHECK(get_default_thread_pool_concurrency() == 2);

		CHECK(!default_thread_pool().is_stopped());
		CHECK(default_thread_pool().workers() == 2);

		auto f = async([](int x){return 42;}, 4);
		CHECK(f.get() == 42);
	}
}
