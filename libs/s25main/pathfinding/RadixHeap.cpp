

#include "RadixHeap.h"
#include "RTTR_Assert.h"
#include "nodeObjs/noRoadNode.h"
#include <algorithm>
#include <iostream>
#include <limits>

#define RADIX_DEBUG

// #ifdef RADIX_DEBUG
// static void print_bu(const std::array<std::vector<const noRoadNode*>, 33>& b, const std::array<unsigned, 33>& u)
// {
//     std::string b_str;
//     std::string u_str;

//     for(unsigned i = 0; i < 33; ++i)
//     {
//         std::string b_s;
//         for(const auto& el: b[i]) {
//             b_s += std::to_string(el->estimate) + " ";
//         }

//         std::string u_s = std::to_string(u[i]);
//         for(unsigned c = u_s.size(); c < b_s.size() - 1; ++c)
//             u_s += " ";

//         b_str += b_s + "|";
//         u_str += u_s + "|";
//     }
//     std::cout << b_str << std::endl;
//     std::cout << u_str << std::endl;
// }
// #endif

const noRoadNode* RadixHeap::delete_min()
{
    RTTR_Assert(!empty());
    size_--;

    const noRoadNode* ret = nullptr;

#ifdef BY_THE_BOOK
    if(!b[0].empty())
    {
        ret = b[0].back();
        b[0].pop_back();
        return ret;
    }

    // Get first non empty bucket
    unsigned i = 0;
    while(b[i].empty())
        i++;
    RTTR_Assert(i < 33);

    // If there was no result in the first bucket, we have to get
    // the minimum fron the first non-empty bucket b[i]
    if(i > 0)
    {
        unsigned k;
        for(unsigned j = 0; j < b[i].size(); ++j)
        {
            if(!ret || T_GetOrderValue::GetValue(b[i][j]) < T_GetOrderValue::GetValue(ret))
            {
                k = j;
                ret = b[i][k];
            }
        }
        b[i].erase(b[i].begin() + k);
        // @todo:
        // elements[bestIdx] = elements.back();
        // elements.pop_back();

        if(b[i].empty())
            return ret;

        // Get smallest key in remaining elements
        unsigned min = std::numeric_limits<unsigned>::max();
        for(T& el : b[i])
            min = std::min(min, T_GetOrderValue::GetValue(el));

        u[0] = min;
        u[1] = u[0] + 1;
        for(unsigned j = 2; j <= i; ++j)
        {
            u[j] = std::min(u[j - 1] + (2 << (j - 2)), u[i + 1]);
        }

        // unsigned j = 0;
        for(T& el : b[i])
        {
            // while(T_GetOrderValue::GetValue(el) > u[j + 1])
            // j++;
            unsigned j = get_bucket(T_GetOrderValue::GetValue(el), el->bucket_);
            RTTR_Assert(i > j);
            b[j].push_back(el);
        }
        b[i].clear();
    }
#else
    if(b[0].empty())
    {
        // Get first non empty bucket
        unsigned i = get_first_non_empty_bucket();
        ret = extract_min(i);

        if(empty())
        {
#ifdef RADIX_DEBUG
    std::cout << "delete_min() -> " << ret->estimate << std::endl;
#endif
            return ret;
        }

        // Which bucket is now the first non empty?
        i = get_first_non_empty_bucket(i);

        // Get smallest key in bucket b[i]
        unsigned min = ret->estimate;
        // unsigned min = std::numeric_limits<unsigned>::max();
        // for(const noRoadNode* el : b[i]) {
        //     min = std::min(min, el->estimate);
        // }

        // Update bounds
        u[0] = min;
        u[1] = u[0] + 1;
        for(unsigned j = 2; j <= i; ++j)
        {
            u[j] = std::min(u[j - 1] + (2 << (j - 2)), u[i + 1]);
        }

        // Redistribute elements
        std::vector<const noRoadNode*> b_i(std::move(b[i]));
        for(const noRoadNode* el : b_i)
        {
            unsigned j = get_bucket(el->estimate, el->bucket_);
            RTTR_Assert(j <= i);
            b[j].push_back(el);
            el->bucket_ = j;
        }
        b_i.clear();
    } else 
    {
	    ret = b[0].back();
	    b[0].pop_back();
    }

#endif

#ifdef RADIX_DEBUG
    std::cout << "delete_min() -> " << ret->estimate << std::endl;
#endif
    return ret;
}

void RadixHeap::clear()
{
    for(auto& bucket : b)
        bucket.clear();
    u[0] = 0;
    u[1] = 1;
    for(unsigned i = 2; i < u.size(); ++i)
        u[i] = 2 << (i - 2);
    size_ = 0;

#ifdef RADIX_DEBUG
    std::cout << "clear()" << std::endl;
#endif
}

void RadixHeap::insert(const noRoadNode* el)
{
    RTTR_Assert(el->estimate >= u[0]);
    unsigned i = get_bucket(el->estimate);
    b[i].push_back(el);
    el->bucket_ = i;
    size_++;

#ifdef RADIX_DEBUG
    std::cout << "insert() -> " << el->estimate << std::endl;
#endif
}

void RadixHeap::decrease_key(const noRoadNode* el)
{
    RTTR_Assert(el->estimate >= u[0]);
    // remove el from current bucket
    unsigned i = el->bucket_;
    RTTR_Assert(i < 33);

    unsigned new_bucket_idx = get_bucket(el->estimate);

    RTTR_Assert(new_bucket_idx <= i);
    if (new_bucket_idx != i)
    {
        b[i].erase(std::remove(b[i].begin(), b[i].end(), el), b[i].end());
        b[new_bucket_idx].push_back(el);
        el->bucket_ = new_bucket_idx;
    }

    // @todo:
    // elements[bestIdx] = elements.back();
    // elements.pop_back();

#ifdef RADIX_DEBUG
    std::cout << "decrease_key() -> " << el->estimate << std::endl;
#endif
}

const noRoadNode* RadixHeap::extract_min(unsigned bucket_idx)
{
    RTTR_Assert(!b[bucket_idx].empty());

    const noRoadNode* ret = b[bucket_idx][0];
    unsigned ret_idx = 0;
    for(unsigned i = 1; i < b[bucket_idx].size(); ++i)
    {
        if(b[bucket_idx][i]->estimate < ret->estimate)
        {
            ret = b[bucket_idx][i];
            ret_idx = i;
        }
    }

    if(b[bucket_idx].size() > 1)
    {
        b[bucket_idx][ret_idx] = b[bucket_idx].back();
        b[bucket_idx].pop_back();
    } else
    {
        b[bucket_idx].clear();
    }

    return ret;
}
