#include <catch2/catch.hpp>
#include <ape/thread/threadpool.hpp>

TEST_CASE("ape::thread::thread_pool", "[ape.thread.pool]"){
	using namespace ape::thread;
	using namespace std::chrono_literals;

	SECTION("Check global thread pool"){
		set_default_thread_pool_concurrency(2);

		CHECK(get_default_thread_pool_concurrency() == 2);

		CHECK(!default_thread_pool().is_stopped());
		CHECK(default_thread_pool().workers() == 2);

		auto f = async([](int a, int b){return a + b;}, 40, 2);
		CHECK(f.get() == 42);
	}
	SECTION("Check thread pool object"){
		auto tpool = thread_pool::create();
		auto f = tpool->async([]{return 42;});
		CHECK(f.get() == 42);

		tpool->stop();
	}
}
