/*	Jake Shoffner
	city_map.cpp
	04/05/22
	Credit to Dr. James Plank (UTK) for "city_map.hpp"
*/
#include "city_map.hpp"
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <set>
using namespace std;

double City_Map::Dijkstra(int avg_best_worst) {
	list <Road_Segment *>::iterator it;
	list <Intersection *>::iterator iit;
	Intersection *node, *node2, *hold;
	Road_Segment *temp;
	double t_time = 0;
	double comp;
	
	/* Setting every single intersection's best_time to 0 */
	for (iit = all.begin(); iit != all.end(); ++iit) {
		hold = *iit;
		hold->best_time = 0;
	}
	
	/* Setting the first intersection's backedge to NULL and best_time to 0, as well as setting its bfsq_ptr to the inserted multimap element */
	first->backedge = NULL;
	first->best_time = 0;
	first->bfsq_ptr = bfsq.insert(make_pair(t_time, first));

	/* Loops until our multimap is empty */
	while (!bfsq.empty()) {
		/* Setting our current node to be the first one in the multimap and erasing it */
		node = bfsq.begin()->second;
		bfsq.erase(node->bfsq_ptr);

		/* We reached the last node, so we push to the front the path from the last node to the first and return the best time */
		if (node == last) {
			while (node != first) {
				path.push_front(node->backedge);
				node = node->backedge->from;
			}
			return last->best_time;
		}

		/* Going through the node's adjacency list */
		for (it = node->adj.begin(); it != node->adj.end(); ++it) {
			temp = *it;
			node2 = temp->to;

			/* Logic for deciding if we want the best, worst, or average time calculation */
			if (avg_best_worst == 'B') t_time = (temp->distance / 30 * 3600);
			else if (avg_best_worst == 'W' && temp->type == STREET) t_time = (temp->distance / 30) * 3600 + temp->to->green[AVENUE];
			else if (avg_best_worst == 'W' && temp->type == AVENUE) t_time = (temp->distance / 30) * 3600 + temp->to->green[STREET];
			else if (avg_best_worst == 'A' && temp->type == STREET) {
				t_time = (temp->distance / 30) * 3600 + (temp->to->green[AVENUE] * temp->to->green[AVENUE]) / (2 * (temp->to->green[AVENUE] + temp->to->green[STREET]));
			}
			else if (avg_best_worst == 'A' && temp->type == AVENUE) {
				t_time = (temp->distance / 30) * 3600 + (temp->to->green[STREET] * temp->to->green[STREET]) / (2 * (temp->to->green[AVENUE] + temp->to->green[STREET]));
			}
			
			/* Updating the comparison variable */
			comp = node->best_time + t_time;

			/* If the comparer is faster than the adjacent node's best time, we remove the adjacent node and update */
			if (comp < node2->best_time || node2->best_time == 0) {
				if (bfsq.find(node2->best_time) != bfsq.end()) bfsq.erase(node2->bfsq_ptr);
				node2->best_time = comp;
				node2->backedge = temp;
				node2->bfsq_ptr = bfsq.insert(make_pair(comp, node2));
			}
		}
	}
	return 0;
}

City_Map::City_Map() {
	Intersection *is;
	Road_Segment *rs, *rs2;
	int st, av;
	double xm, ym, gs, ga, dist, pow1, pow2;
	map <int, map <int, Intersection *> > ave, str;
	map <int, map <int, Intersection *> >::iterator mit;
	map <int, Intersection *>::iterator lmit, lmit2;
	list <Intersection *>::iterator it;
	Intersection *temp;

	/* Reading in every intersection and defining each field */
	while (cin >> st) {
		cin >> av >> xm >> ym >> gs >> ga;

		is = new Intersection;
		is->street = st;
		is->avenue = av;
		is->x = xm;
		is->y = ym;
		is->green[STREET] = gs;
		is->green[AVENUE] = ga;
		
		/* Pushing back the intersection in the 'all' list for intersections */
		all.push_back(is);

		/* Temporarily storing them in 2 maps for adjacency list ease */
		str[is->street][is->avenue] = is;
		ave[is->avenue][is->street] = is;
	}

	/* Going through the entire temporary map keyed on avenue */
	for (mit = ave.begin(); mit != ave.end(); ++mit) {
		for (lmit = mit->second.begin(); lmit != mit->second.end(); ++lmit) {
			/* Basically, we are seeing if this is the last element, if it's not, we can increment the other pointer */
			lmit2 = mit->second.end();
			lmit2--;
			if (lmit != lmit2) {
				lmit2 = lmit;
				lmit2++;

				pow1 = lmit->second->x - lmit2->second->x;
				pow2 = lmit->second->y - lmit2->second->y;
				dist = sqrt((pow1 * pow1) + (pow2 * pow2));
				
				/* If the Avenue is the highest numbered or if it's a multiple of 5, it's two-way */
				if (mit->first == ave.rbegin()->first || mit->first % 5 == 0) {
					/* Road segment 1, from first to second avenue we are observing */
					rs = new Road_Segment;
					rs->distance = dist;
					rs->type = AVENUE;
					rs->number = mit->first;
					rs->from = lmit->second;
					rs->to = lmit2->second;
					
					/* Road segment 2, from second to first avenue we are observing */
					rs2 = new Road_Segment;
					rs2->distance = dist;
					rs2->type = AVENUE;
					rs2->number = mit->first;
					rs2->from = lmit2->second;
					rs2->to = lmit->second;
					
					/* Finding the Intersection in 'all' and pushing back the first road segment to the adjacency list */
					it = find(all.begin(), all.end(), rs->from);
					temp = *it;
					temp->adj.push_back(rs);
					
					/* Finding the Intersection in 'all' and pushing back the second road segment to the adjacency list */
					it = find(all.begin(), all.end(), rs2->from);
					temp = *it;
					temp->adj.push_back(rs2);
				}
				/* Avenue is EVEN = North to South */
				else if (mit->first % 2 == 0) {
					rs = new Road_Segment;
					rs->distance = dist;
					rs->type = AVENUE;
					rs->number = mit->first;
					rs->from = lmit->second;
					rs->to = lmit2->second;
					
					it = find(all.begin(), all.end(), rs->from);
					temp = *it;
					temp->adj.push_back(rs);
				}
				/* Avenue is ODD = South to North */
				else if (mit->first % 2 == 1){
					rs = new Road_Segment;
					rs->distance = dist;
					rs->type = AVENUE;
					rs->number = mit->first;
					rs->from = lmit2->second;
					rs->to = lmit->second;
					
					it = find(all.begin(), all.end(), rs->from);
					temp = *it;
					temp->adj.push_back(rs);
				}
			}
		}
	} 
	
	for (mit = str.begin(); mit != str.end(); ++mit) {
		for (lmit = mit->second.begin(); lmit != mit->second.end(); ++lmit) {
			/* Basically, we are seeing if this is the last element, if it's not, we can increment the other pointer */
			lmit2 = mit->second.end();
			lmit2--;
			if (lmit != lmit2) {
				lmit2 = lmit;
				lmit2++;

				pow1 = lmit->second->x - lmit2->second->x;
				pow2 = lmit->second->y - lmit2->second->y;
				dist = sqrt((pow1 * pow1) + (pow2 * pow2));
				
				/* If the Street is a multiple of 5, it's two-way */
				if (mit->first % 5 == 0) {
					/* Road segment 1, from first to second street we are observing */
					rs = new Road_Segment;
					rs->distance = dist;
					rs->type = STREET;
					rs->number = mit->first;
					rs->from = lmit->second;
					rs->to = lmit2->second;
					
					/* Road segment 2, from second to first street we are observing */
					rs2 = new Road_Segment;
					rs2->distance = dist;
					rs2->type = STREET;
					rs2->number = mit->first;
					rs2->from = lmit2->second;
					rs2->to = lmit->second;
					
					/* Finding the Intersection in 'all' and pushing back the first road segment to the adjacency list */
					it = find(all.begin(), all.end(), rs->from);
					temp = *it;
					temp->adj.push_back(rs);
					
					/* Finding the Intersection in 'all' and pushing back the second road segment to the adjacency list */
					it = find(all.begin(), all.end(), rs2->from);
					temp = *it;
					temp->adj.push_back(rs2);
				}
				/* Street is EVEN = East to West */
				else if (mit->first % 2 == 0) {
					rs = new Road_Segment;
					rs->distance = dist;
					rs->type = STREET;
					rs->number = mit->first;
					rs->from = lmit->second;
					rs->to = lmit2->second;
					
					it = find(all.begin(), all.end(), rs->from);
					temp = *it;
					temp->adj.push_back(rs);
				}
				/* Street is ODD = West to East */
				else if (mit->first % 2 == 1){
					rs = new Road_Segment;
					rs->distance = dist;
					rs->type = STREET;
					rs->number = mit->first;
					rs->from = lmit2->second;
					rs->to = lmit->second;
					
					it = find(all.begin(), all.end(), rs->from);
					temp = *it;
					temp->adj.push_back(rs);
				}
			}
		}
	}

	/* Setting first and last pointers */
	last = ave.rbegin()->second.rbegin()->second;
	first = ave.begin()->second.begin()->second;
}
