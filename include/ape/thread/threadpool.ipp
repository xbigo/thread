#ifndef APE_THREAD_THREAD_POOL_HPP
#error "This file intends to be included by threadpool.hpp only" 
#endif


BEGIN_APE_NAMESPACE
namespace thread{
	namespace detail {
		template<typename R> struct invoker{
			template<typename F, typename ... Args>
				static void invoke(std::promise<R>& p, F&& f_, Args&&... args_){
					p.set_value(f_(std::forward<Args>(args_)...));
				}
		};

		template<> struct invoker<void> {
			template<typename F, typename ... Args>
				static void invoke(std::promise<void>& p, F&& f_, Args&&... args_){
					f_(std::forward<Args>(args_)...);
					p.set_value();
				}
		};
	}

	template<typename F, typename ...Args>
		std::future<detail::result_type<F, Args...>> thread_pool::async(F&& f, Args&&... args) {
			using R = detail::result_type<F, Args...>;
			auto call = [](std::promise<R> p, F&& f_, Args&&... args_){
				try{
					detail::invoker<R>::invoke(p, std::forward<F>(f_), std::forward<Args>(args_)...);
				}
				catch(...){
					p.set_exception(std::current_exception());
				}
			};
			std::promise<R> p;
			auto future = p.get_future();
			enqueue_(std::bind(call, std::move(p), std::forward<F>(f), std::forward<Args>(args)...));
			return future;
		}

}
END_APE_NAMESPACE

