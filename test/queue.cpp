#include <catch2/catch.hpp>
#include <ape/thread/queue.hpp>
#include <thread>
#include <atomic>
#include <deque>
#include <future>


TEST_CASE("ape::thread::queue", "[ape.thread.queue]"){
	using ape::thread::queue;
	using namespace std::chrono_literals;

	SECTION("Check default ctor")
	{
		queue<int> queue;
		CHECK(queue.is_empty());

		CHECK(queue.is_empty());
#if 1
		std::thread th([&]{
				std::this_thread::sleep_for(10ms);
				queue.stop();
				});
#endif
		CHECK(!queue.wait()); //false means the queue is empty
#if 1
		th.join();
#endif
	}

	SECTION("Check push and pop")
	{
		queue<int> queue;
		queue.push_back(1);
		queue.push_back(2);

		CHECK(!queue.is_empty());

		CHECK(queue.wait());
		CHECK(!queue.is_empty());

		int output_val;
		CHECK(queue.wait_and_pop(output_val));
		CHECK(output_val == 1);
		CHECK(!queue.is_empty());
		CHECK(queue.wait_and_pop(output_val));
		CHECK(output_val == 2);
		CHECK(queue.is_empty());


		queue.push_back(3);
		queue.push_front(4);
		CHECK(!queue.is_empty());

		CHECK(queue.wait_and_pop(output_val));
		CHECK(!queue.is_empty());

		CHECK(output_val == 4);
	}

	SECTION("Check utilities")
	{
		queue<int> queue;
		queue.push_back(1);
		queue.push_back(2);
		CHECK(queue.size() == 2);

		std::deque<int> deque_take_all;
		queue.take_all(deque_take_all);
		CHECK(deque_take_all.size() == 2);
		CHECK(queue.is_empty());

		queue.push_back(1);
		queue.push_back(2);
		CHECK(!queue.is_empty());
		queue.clear();
		CHECK(queue.is_empty());

		queue.push_back(1);
		queue.push_back(2);
		queue.stop();
		CHECK(!queue.push_back(3));

		queue.reset();
		CHECK(queue.push_back(3));
	}



	SECTION("Check against thread")
	{
		queue<int> queue;
		{
			int output_val;
			std::atomic<int> counter{ 0 };
			auto future = std::async([&] {
					queue.wait_and_pop(output_val, 1s);
					++counter;
					});

			queue.stop();
			future.get();
			CHECK(counter.load() == 1);
		}
		{
			int output_val;
			std::atomic<int> counter{ 0 };
			auto future = std::async([&] {
					queue.wait_and_pop(output_val, 1s);
					++counter;
					});

			queue.push_back(1);
			future.get();
			CHECK(counter.load() == 1);
		}

	}
}
