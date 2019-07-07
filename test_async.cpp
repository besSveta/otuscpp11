/*
 * test_bulk.cpp
 *
 *  Created on: 29 июн. 2019 г.
 *      Author: sveta
 */

#define BOOST_TEST_MODULE ip_filter_test_module
#include <boost/test/included/unit_test.hpp>
#include "async.h"
#include "bulkasync.h"

BOOST_AUTO_TEST_SUITE(async_test_suite)

BOOST_AUTO_TEST_CASE(async__some_test_case)
{
	  std::size_t bulk = 5;
	    auto h = async::connect(bulk);
	    async::receive(h, "1", 1);
	    BOOST_REQUIRE_EQUAL("WaitComand", async::GetState(h));
	    async::disconnect(h);

}

BOOST_AUTO_TEST_SUITE_END()





