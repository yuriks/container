//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2004-2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_CONTAINER_TEST_VECTOR_TEST_HEADER
#define BOOST_CONTAINER_TEST_VECTOR_TEST_HEADER

#include <boost/container/detail/config_begin.hpp>

#include <vector>
#include <iostream>
#include <list>

#include <boost/move/utility_core.hpp>
#include <boost/container/detail/mpl.hpp>
#include <boost/move/utility_core.hpp>
#include <boost/move/iterator.hpp>
#include <boost/move/make_unique.hpp>
#include <boost/core/no_exceptions_support.hpp>
#include <boost/static_assert.hpp>

#include "print_container.hpp"
#include "check_equal_containers.hpp"
#include "movable_int.hpp"
#include "emplace_test.hpp"
#include "input_from_forward_iterator.hpp"
#include "insert_test.hpp"
#include "container_common_tests.hpp"

#include <cstddef>
#include <string>
#include <vector>


namespace boost{
namespace container {
namespace test{

template<class V1, class V2>
bool vector_copyable_only(V1&, V2&, boost::container::container_detail::false_type)
{
   return true;
}

//Function to check if both sets are equal
template<class V1, class V2>
bool vector_copyable_only(V1 &boostvector, V2 &stdvector, boost::container::container_detail::true_type)
{
   typedef typename V1::value_type IntType;
   std::size_t size = boostvector.size();
   boostvector.insert(boostvector.end(), 50, IntType(1));
   stdvector.insert(stdvector.end(), 50, 1);
   if(!test::CheckEqualContainers(boostvector, stdvector)) return false;

   {
      IntType move_me(1);
      boostvector.insert(boostvector.begin()+size/2, 50, boost::move(move_me));
      stdvector.insert(stdvector.begin()+size/2, 50, 1);
      if(!test::CheckEqualContainers(boostvector, stdvector)) return false;
   }
   {
      IntType move_me(2);
      boostvector.assign(boostvector.size()/2, boost::move(move_me));
      stdvector.assign(stdvector.size()/2, 2);
      if(!test::CheckEqualContainers(boostvector, stdvector)) return false;
   }
   {
      IntType move_me(3);
      boostvector.assign(boostvector.size()*3-1, boost::move(move_me));
      stdvector.assign(stdvector.size()*3-1, 3);
      if(!test::CheckEqualContainers(boostvector, stdvector)) return false;
   }

   {
      IntType copy_me(3);
      const IntType ccopy_me(3);
      boostvector.push_back(copy_me);
      stdvector.push_back(int(3));
      boostvector.push_back(ccopy_me);
      stdvector.push_back(int(3));
      if(!test::CheckEqualContainers(boostvector, stdvector)) return false;
   }
   {
      ::boost::movelib::unique_ptr<V1> const pv1 = ::boost::movelib::make_unique<V1>(boostvector);
      ::boost::movelib::unique_ptr<V2> const pv2 = ::boost::movelib::make_unique<V2>(stdvector);

      V1 &v1 = *pv1;
      V2 &v2 = *pv2;

      boostvector.clear();
      stdvector.clear();
      boostvector.assign(v1.begin(), v1.end());
      stdvector.assign(v2.begin(), v2.end());
      if(!test::CheckEqualContainers(boostvector, stdvector)) return 1;
   }

   return true;
}

template<class MyBoostVector>
int vector_test()
{
   typedef std::vector<int>                     MyStdVector;
   typedef typename MyBoostVector::value_type   IntType;
   const int max = 100;

   if(!test_range_insertion<MyBoostVector>()){
      return 1;
   }

   {
      ::boost::movelib::unique_ptr<MyBoostVector> const boostvectorp = ::boost::movelib::make_unique<MyBoostVector>();
      ::boost::movelib::unique_ptr<MyStdVector> const stdvectorp = ::boost::movelib::make_unique<MyStdVector>();

      MyBoostVector & boostvector = *boostvectorp;
      MyStdVector & stdvector = *stdvectorp;

      boostvector.resize(100);
      stdvector.resize(100);
      if(!test::CheckEqualContainers(boostvector, stdvector)) return 1;

      boostvector.resize(200);
      stdvector.resize(200);
      if(!test::CheckEqualContainers(boostvector, stdvector)) return 1;

      boostvector.resize(0);
      stdvector.resize(0);
      if(!test::CheckEqualContainers(boostvector, stdvector)) return 1;

      for(int i = 0; i < max; ++i){
         IntType new_int(i);
         boostvector.insert(boostvector.end(), boost::move(new_int));
         stdvector.insert(stdvector.end(), i);
         if(!test::CheckEqualContainers(boostvector, stdvector)) return 1;
      }
      if(!test::CheckEqualContainers(boostvector, stdvector)) return 1;

      typename MyBoostVector::iterator boostit(boostvector.begin());
      typename MyStdVector::iterator stdit(stdvector.begin());
      typename MyBoostVector::const_iterator cboostit = boostit;
      (void)cboostit;
      ++boostit; ++stdit;
      boostvector.erase(boostit);
      stdvector.erase(stdit);
      if(!test::CheckEqualContainers(boostvector, stdvector)) return 1;

      boostvector.erase(boostvector.begin());
      stdvector.erase(stdvector.begin());
      if(!test::CheckEqualContainers(boostvector, stdvector)) return 1;

      {
         //Initialize values
         IntType aux_vect[50];
         for(int i = 0; i < 50; ++i){
            IntType new_int(-1);
            BOOST_STATIC_ASSERT((boost::container::test::is_copyable<boost::container::test::movable_int>::value == false));
            aux_vect[i] = boost::move(new_int);
         }
         int aux_vect2[50];
         for(int i = 0; i < 50; ++i){
            aux_vect2[i] = -1;
         }
         typename MyBoostVector::iterator insert_it =
            boostvector.insert(boostvector.end()
                           ,boost::make_move_iterator(&aux_vect[0])
                           ,boost::make_move_iterator(aux_vect + 50));
         if(std::size_t(boost::container::iterator_distance(insert_it, boostvector.end())) != 50) return 1;
         stdvector.insert(stdvector.end(), aux_vect2, aux_vect2 + 50);
         if(!test::CheckEqualContainers(boostvector, stdvector)) return 1;

         for(int i = 0, j = static_cast<int>(boostvector.size()); i < j; ++i){
            boostvector.erase(boostvector.begin());
            stdvector.erase(stdvector.begin());
         }
         if(!test::CheckEqualContainers(boostvector, stdvector)) return 1;
      }
      {
         boostvector.resize(100);
         stdvector.resize(100);
         if(!test::CheckEqualContainers(boostvector, stdvector)) return 1;

         IntType aux_vect[50];
         for(int i = 0; i < 50; ++i){
            IntType new_int(-i);
            aux_vect[i] = boost::move(new_int);
         }
         int aux_vect2[50];
         for(int i = 0; i < 50; ++i){
            aux_vect2[i] = -i;
         }
         typename MyBoostVector::size_type old_size = boostvector.size();
         typename MyBoostVector::iterator insert_it =
            boostvector.insert(boostvector.begin() + old_size/2
                           ,boost::make_move_iterator(&aux_vect[0])
                           ,boost::make_move_iterator(aux_vect + 50));
         if(boostvector.begin() + old_size/2 != insert_it) return 1;
         stdvector.insert(stdvector.begin() + old_size/2, aux_vect2, aux_vect2 + 50);
         if(!test::CheckEqualContainers(boostvector, stdvector)) return 1;

         for(int i = 0; i < 50; ++i){
            IntType new_int(-i);
            aux_vect[i] = boost::move(new_int);
         }

         for(int i = 0; i < 50; ++i){
            aux_vect2[i] = -i;
         }
         old_size = boostvector.size();
         //Now try with input iterators instead
         insert_it = boostvector.insert(boostvector.begin() + old_size/2
                           ,boost::make_move_iterator(make_input_from_forward_iterator(&aux_vect[0]))
                           ,boost::make_move_iterator(make_input_from_forward_iterator(aux_vect + 50))
                        );
         if(boostvector.begin() + old_size/2 != insert_it) return 1;
         stdvector.insert(stdvector.begin() + old_size/2, aux_vect2, aux_vect2 + 50);
         if(!test::CheckEqualContainers(boostvector, stdvector)) return 1;
      }
/*       //deque has no reserve
      boostvector.reserve(boostvector.size()*2);
      stdvector.reserve(stdvector.size()*2);
      if(!test::CheckEqualContainers(boostvector, stdvector)) return 1;
*/
      boostvector.shrink_to_fit();
      MyStdVector(stdvector).swap(stdvector);
      if(!test::CheckEqualContainers(boostvector, stdvector)) return 1;

      boostvector.shrink_to_fit();
      MyStdVector(stdvector).swap(stdvector);
      if(!test::CheckEqualContainers(boostvector, stdvector)) return 1;

      {  //push_back with not enough capacity
      IntType push_back_this(1);
      boostvector.push_back(boost::move(push_back_this));
      stdvector.push_back(int(1));
      boostvector.push_back(IntType(1));
      stdvector.push_back(int(1));
      if(!test::CheckEqualContainers(boostvector, stdvector)) return 1;
      }

      {  //test back()
      const IntType test_this(1);
      if(test_this != boostvector.back())   return 1;
      }
      {  //pop_back with enough capacity
      boostvector.pop_back();
      boostvector.pop_back();
      stdvector.pop_back();
      stdvector.pop_back();

      IntType push_back_this(1);
      boostvector.push_back(boost::move(push_back_this));
      stdvector.push_back(int(1));
      boostvector.push_back(IntType(1));
      stdvector.push_back(int(1));
      if(!test::CheckEqualContainers(boostvector, stdvector)) return 1;
      }

      if(!vector_copyable_only(boostvector, stdvector
                     ,container_detail::bool_<boost::container::test::is_copyable<IntType>::value>())){
         return 1;
      }

      boostvector.erase(boostvector.begin());
      stdvector.erase(stdvector.begin());
      if(!test::CheckEqualContainers(boostvector, stdvector)) return 1;

      for(int i = 0; i < max; ++i){
         IntType insert_this(i);
         boostvector.insert(boostvector.begin(), boost::move(insert_this));
         stdvector.insert(stdvector.begin(), i);
         boostvector.insert(boostvector.begin(), IntType(i));
         stdvector.insert(stdvector.begin(), int(i));
      }
      if(!test::CheckEqualContainers(boostvector, stdvector)) return 1;

      //some comparison operators
      if(!(boostvector == boostvector))
         return 1;
      if(boostvector != boostvector)
         return 1;
      if(boostvector < boostvector)
         return 1;
      if(boostvector > boostvector)
         return 1;
      if(!(boostvector <= boostvector))
         return 1;
      if(!(boostvector >= boostvector))
         return 1;

      //Test insertion from list
      {
         std::list<int> l(50, int(1));
         typename MyBoostVector::iterator it_insert =
            boostvector.insert(boostvector.begin(), l.begin(), l.end());
         if(boostvector.begin() != it_insert) return 1;
         stdvector.insert(stdvector.begin(), l.begin(), l.end());
         if(!test::CheckEqualContainers(boostvector, stdvector)) return 1;
         boostvector.assign(l.begin(), l.end());
         stdvector.assign(l.begin(), l.end());
         if(!test::CheckEqualContainers(boostvector, stdvector)) return 1;

         boostvector.clear();
         stdvector.clear();
         boostvector.assign(make_input_from_forward_iterator(l.begin()), make_input_from_forward_iterator(l.end()));
         stdvector.assign(l.begin(), l.end());
         if(!test::CheckEqualContainers(boostvector, stdvector)) return 1;
      }

      boostvector.resize(100);
      if(!test_nth_index_of(boostvector))
         return 1;
/*       deque has no reserve or capacity
      std::size_t cap = boostvector.capacity();
      boostvector.reserve(cap*2);
      stdvector.reserve(cap*2);
      if(!test::CheckEqualContainers(boostvector, stdvector)) return 1;
      boostvector.resize(0);
      stdvector.resize(0);
      if(!test::CheckEqualContainers(boostvector, stdvector)) return 1;

      boostvector.resize(cap*2);
      stdvector.resize(cap*2);
      if(!test::CheckEqualContainers(boostvector, stdvector)) return 1;

      boostvector.clear();
      stdvector.clear();
      boostvector.shrink_to_fit();
      MyStdVector(stdvector).swap(stdvector);
      if(!test::CheckEqualContainers(boostvector, stdvector)) return 1;

      boostvector.resize(cap*2);
      stdvector.resize(cap*2);
      if(!test::CheckEqualContainers(boostvector, stdvector)) return 1;
*/
   }
   std::cout << std::endl << "Test OK!" << std::endl;
   return 0;
}

template<typename VectorContainerType>
bool test_vector_methods_with_initializer_list_as_argument_for()
{
#if !defined(BOOST_NO_CXX11_HDR_INITIALIZER_LIST)
   {
      const VectorContainerType testedVector = {1, 2, 3};
      const std::vector<int> expectedVector = {1, 2, 3};
      if(!test::CheckEqualContainers(testedVector, expectedVector)) return false;
   }

   {
      VectorContainerType testedVector = {1, 2, 3};
      testedVector = {11, 12, 13};

      const std::vector<int> expectedVector = {11, 12, 13};
      if(!test::CheckEqualContainers(testedVector, expectedVector)) return false;
   }

   {
      VectorContainerType testedVector = {1, 2, 3};
      testedVector.assign({5, 6, 7});

      const std::vector<int> expectedVector = {5, 6, 7};
      if(!test::CheckEqualContainers(testedVector, expectedVector)) return false;
   }

   {
      VectorContainerType testedVector = {1, 2, 3};
      testedVector.insert(testedVector.cend(), {5, 6, 7});

      const std::vector<int> expectedVector = {1, 2, 3, 5, 6, 7};
      if(!test::CheckEqualContainers(testedVector, expectedVector)) return false;
   }
   return true;
#else
   return true;
#endif
}

}  //namespace test{
}  //namespace container {
}  //namespace boost{

#include <boost/container/detail/config_end.hpp>

#endif //BOOST_CONTAINER_TEST_VECTOR_TEST_HEADER
