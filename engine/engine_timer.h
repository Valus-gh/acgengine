/**
 * @file		engine_timer.h
 * @brief	Timing utils
 *
 * @author	Achille Peternier (achille.peternier@supsi.ch), (C) SUPSI
 */
#pragma once


/**
 * @brief Timer class. This class is a singleton.
 */
class ENG_API Timer
{
	//////////
public: //
	//////////

	// Const/dest:   
	Timer(const Timer&) = delete;
	~Timer();

	// Get/set:
	uint64_t getCounter() const;
	double getCounterDiff(uint64_t t1, uint64_t t2) const;

	// Operators:
	void operator=(const Timer&) = delete;

	// Singleton:
	static Timer& getInstance();


	///////////
private: //
	///////////

	// Reserved:
	struct Reserved;
	std::unique_ptr<Reserved> reserved;

	// Const/dest:
	Timer();
};
