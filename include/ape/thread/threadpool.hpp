#ifndef APE_THREAD_THREAD_POOL_HPP
#define APE_THREAD_THREAD_POOL_HPP
#include <ape/thread/queue.hpp>

#include <functional>
#include <thread>
#include <memory>
#include <future>
#include <type_traits>

BEGIN_APE_NAMESPACE
namespace thread{
	namespace detail{
		template<typename F, typename ... Args>
			using result_type = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>;
	}

	class APE_API thread_pool
	{
			void enqueue_(std::function<void()> t);
			thread_pool(const thread_pool&) = delete;
			thread_pool& operator=(const thread_pool&) = delete;
		public:
			typedef std::function<void()> task_type;

			/// thread safe
			bool is_stopped() const;

			/// thread safe
			std::size_t workers() const;

			/// stop background workers. thread unsafe
			/// \return false if already stopped
			/// \note stop won't clear the pending tasks
			bool stop();
			/// clear pending tasks
			/// thread safe
			void clear();
			/// start workers. thread unsafe
			/// \return false if already started
			bool start();

			/// get underlying queue, debug purpose
			/// thread safe
			const queue<task_type>& get_queue() const;
			/// get underlying queue, debug purpose
			/// thread safe
			queue<task_type>& get_queue();

			/// launch an async task call
			/// thread safe
			template<typename F, typename ...Args>
				std::future<detail::result_type<F, Args...>> async(F&& f, Args&&... args);

			/// create a new thread pool
			/// thread safe
			static std::shared_ptr<thread_pool> create(bool auto_start = true, std::size_t count = std::thread::hardware_concurrency());
		protected:
			thread_pool();
			~thread_pool();
	};

	/// thread safe
	APE_API extern thread_pool& default_thread_pool();
	/// thread unsafe.
	/// \pre default_thread_pool().is_stopped() or before calling default_thread_pool()
	APE_API extern void set_default_thread_pool_concurrency(std::size_t count);

	/// thread safe if no set_default_thread_pool_concurrency is calling
	APE_API extern std::size_t get_default_thread_pool_concurrency();

	/// thread safe
	template<typename F, typename ...Args>
		inline std::future<detail::result_type<F, Args...>> async(F&& f, Args&&... args){
			auto& tpool = default_thread_pool();
			return tpool.async(std::forward<F>(f), std::forward<Args>(args)...);
		}
}
END_APE_NAMESPACE

#include <ape/thread/threadpool.ipp>
#endif //end APE_THREAD_THREAD_POOL_HPP

