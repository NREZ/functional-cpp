#include "proto.hpp"
#include "show.hpp"
#include "functor.hpp"
#include "applicative_functor.hpp"
#include "monad.hpp"
#include "state_monad.hpp"


typedef state_computation<int, std::list<int>> stack_comp;
typedef std::list<int>  istack;



int stm_0()
{
	std::default_random_engine de;
	std::uniform_int_distribution<int> di(10, 20);
	state_computation<int,std::uniform_int_distribution<int>>  getrand = [&de] (std::uniform_int_distribution<int> s) {
		auto val = s(de);
		return state_tuple<int, std::uniform_int_distribution<int> >(val, s);
	}; 

	state <int, std::uniform_int_distribution<int>> ST(getrand);

	std::function<char(int)> f = [] (int i) {
		if (i < 15) {
			return 'A';
		}
		return 'Z';
	};


	int n = 10;
	while (n-- > 0) { 
		auto SRT = functor<state>::fmap(f, ST);
		auto S = runState(SRT, di);
		std::cerr << S << std::endl;;
	}

	//auto SRT = functor<state>::fmap<std::uniform_int_distribution<int>>(f)(ST);
	auto SRT = functor<state>::fmap<std::uniform_int_distribution<int>>(f, ST);
	auto S = runState(SRT, di);
	std::cerr << S << std::endl;;

	return 0;
}



int stm_1()
{
	istack L = {1,2,3,4};

	std::function<char(int)> f = [] (int i) {
		if (i < 3) {
			return 'A';
		}
		return 'Z';
	};

	
	stack_comp pop = [] (istack s) {
		auto val = s.front();
		s.pop_front();
		return state_tuple<int, istack>(val, s);
	};
	
	state <int, istack> SM(pop);
	std::cerr << SM << std::endl;
	state <char, istack> SMT = functor<state>::fmap(f, SM);
	std::cerr << SMT << std::endl;
	
	auto R = runState(SMT, L);
	std::cerr << R << std::endl;


	std::function<stack_comp(int)> push = [](int val) {
		return [val] (istack s) {
			s.push_front(val);
			return state_tuple<int, istack>(s);
		};
	};

	state <int, istack> SP(push(498));
	std::cerr << SP << std::endl;
	auto S = runState(SP, L);
	std::cerr << S << std::endl;

	state <char, istack> SPT = functor<state>::fmap(f, SP);
	auto st = runState(SPT, L);
	std::cerr << st << std::endl;
	
	return 0;
}

int stm_2()
{
	istack L = {1,2,3,4};
	
	auto ST = applicative_functor<state>::pure<istack>(5);
	std::cerr << ST << std::endl;
	
	auto S = runState(ST, L);
	std::cerr << S << std::endl;;

	return 0;
}


int stm_3()
{
	istack L = {1,2,3,4};
	
	std::function<char(int)> f = [] (int i) {
		if (i < 3) {
			return 'A';
		}
		return 'Z';
	};

	stack_comp pop = [] (istack s) {
		auto val = s.front();
		s.pop_front();
		return state_tuple<int, istack>(val, s);
	};
	
	state <int, istack> SM(pop);

	auto F = applicative_functor<state>::pure<istack>(f);
	std::cerr << F << std::endl;

	auto r1 = runState(SM, L);
	std::cerr << r1 << std::endl;;
	auto SC = applicative_functor<state>::apply<istack, int, char>(F, SM);
	auto r2 = runState(SC, L);
	std::cerr << r2 << std::endl;;

	return 0;
}

template<> struct monad<state> : public applicative_functor<state> {
	
	template<typename S, typename A, typename B>
	static std::function < state<B,S> (std::function< state<B,S> (A) > ) > bind(state<A,S> M) {
		return [M](std::function<state<B,S> (A)> f) {
			state_computation<B,S> comp =[f,&M](S s) {
				auto res  = runState(M, s);
				auto valr = res.value();
				if (valr.second) {
					A a         = valr.first;
					S new_state = res.state().first;
					state<B,S> state_g   = f (a);
					return runState(state_g, new_state);
				}
				return state_tuple<B, S>(s);
			};
			return state<B,S> (comp);
		};
	}

	template<typename S, typename A, typename B>
	static state<B,S> bind(state<A,S> M, std::function< state<B,S> (A)> f) {
		state_computation<B,S> comp =[f,&M](S s) {
			auto res  = runState(M, s);
			auto valr = res.value();
			if (valr.second) {
				A a         = valr.first;
				S new_state = res.state().first;
				state<B,S> state_g   = f (a);
				return runState(state_g, new_state);
			}
			return state_tuple<B, S>(s);
		};
		return state<B,S> (comp);
	}
	
	template <typename S, typename A> static state<A,S> mreturn (A val) {
		return applicative_functor<state>::pure<S,A>(val);
	}

};


int stm_4()
{
	istack L = {1,2,3,4};


	std::function< state<int, istack> (int)> f = [] (int v) {
		std::function<stack_comp(int)> push = [](int val) {
			return [val] (istack s) {
				s.push_front(val);
				return state_tuple<int, istack>(s);
			};
		};
		std::cerr << "v : " << v << std::endl;
		if (v > 3) {
			state <int, istack> SP(push(v));		
			return SP;
		}
		stack_comp pop = [] (istack s) {
			auto val = s.front();
			s.pop_front();
			return state_tuple<int, istack>(val, s);
		};
		state <int, istack> SP(pop);		
		return SP;
	};

	stack_comp pop = [] (istack s) {
		auto val = s.front();
		s.pop_front();
		return state_tuple<int, istack>(val, s);
	};
	
	state <int, istack> SM(pop);

	auto R = monad<state>::bind<istack,int,int>(SM, f);
	std::cerr << R << std::endl;

	auto r0 = runState(SM, istack{1,4,5,3,1,8});
	std::cerr << r0 << std::endl;

	auto r1 = runState(R, istack{1,4,5,3,1,8});
	std::cerr << r1 << std::endl;

	return 0;
}

