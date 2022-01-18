#pragma once

#include <stdexcept>

namespace chef {
	class chef_error : public std::runtime_error {
	public:
		using std::runtime_error::runtime_error;
	};

	// An error in constructing one of the chef constructs.
	// (e.g. a DFA, an LL(1) grammar, an LR(1) grammar).
	class construction_error : public chef_error {
	public:
		using chef_error::chef_error;
	};

	// An error in evaluating one of the chef construct.
	// (e.g. a parse error).
	class evaluation_error : public chef_error {
	public:
		using chef_error::chef_error;
	};
}
