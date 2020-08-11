#include <ape/thread/threadpool.hpp>
#include <gsl/gsl_assert>

BEGIN_APE_NAMESPACE
namespace thread{
	struct thread_pool_imp : thread_pool
	{
		bool m_join_in_dtor = true; //TODO: in case ExitProcess or similiars 
		std::size_t m_count = std::thread::hardware_concurrency();
		queue<task_type> m_tasks;
		std::vector<std::thread> m_threads;

		~thread_pool_imp(){
			m_tasks.clear();
			m_tasks.stop();

			if (m_join_in_dtor)
				for(auto& t : m_threads)
					t.join();
		}
	};
	static thread_pool_imp& to_self(thread_pool* this_){
		return *static_cast<thread_pool_imp*>(this_);
	}
	static const thread_pool_imp& to_self(const thread_pool* this_){
		return *static_cast<const thread_pool_imp*>(this_);
	}

	void thread_pool::enqueue_(task_type t){
		auto& self = to_self(this);
		self.m_tasks.emplace_back(std::move(t));
	}
	bool thread_pool::is_stopped() const{
		auto& self = to_self(this);
		return self.m_tasks.is_stopped();
	}
	std::size_t thread_pool::workers() const{
		return to_self(this).m_count;
	}
	bool thread_pool::stop(){
		auto& self = to_self(this);
		if (self.m_tasks.is_stopped())
			return false;
		self.m_tasks.stop();
		for(auto& t : self.m_threads)
			t.join();
		self.m_threads.clear();

		return true;
	}
	void thread_pool::clear(){
		auto& self = to_self(this);
		self.m_tasks.clear();
	}
	bool thread_pool::start(){
		auto self = &to_self(this);
		if (!self->m_tasks.is_stopped())
			return false;

		auto worker = [self]{
			while(!self->m_tasks.is_stopped()){
				try{
					task_type f;
					if (self->m_tasks.wait_and_pop(f) && f)
						f();
				}
				catch(...)
				{}
			}
		};

		for(auto& t : self->m_threads)
			t.join();
		self->m_threads.clear();
		for(std::size_t i = 0; i < self->m_count; ++i){
			self->m_threads.emplace_back(worker);
		}
		return true;
	}
	const queue<thread_pool::task_type>& thread_pool::get_queue() const{
		auto& self = to_self(this);
		return self.m_tasks;
	}
	queue<thread_pool::task_type>& thread_pool::get_queue(){
		auto& self = to_self(this);
		return self.m_tasks;
	}
	thread_pool::thread_pool() = default;
	thread_pool::~thread_pool() = default;

	std::shared_ptr<thread_pool> create(bool auto_start, std::size_t count){
		auto ret = std::make_shared<thread_pool_imp>();
		ret->m_count = std::max(count, std::size_t(1));
		if (auto_start) ret->start();

		return ret;
	}

	static thread_pool_imp g_global_thread_pool;
	APE_API thread_pool& default_thread_pool(){
		static const bool first_init = g_global_thread_pool.start();
		return g_global_thread_pool;
	}

	APE_API void set_default_thread_pool_concurrency(std::size_t count){
		Expects(g_global_thread_pool.is_stopped() || g_global_thread_pool.m_threads.empty());
		g_global_thread_pool.m_count = std::max(count, std::size_t(1));
	}

	APE_API std::size_t get_default_thread_pool_concurrency(){
		return g_global_thread_pool.m_count;
	}
}
END_APE_NAMESPACE

