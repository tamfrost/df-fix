#ifndef WEBASSEMBLY_DIRECTIONCROSSCOLLECTION_H
#define WEBASSEMBLY_DIRECTIONCROSSCOLLECTION_H

#include <unordered_map>
#include <numeric>
#include "geolib.h"
#include "DirectionCross.h"
#include "DirectionCollection.h"

/**
 *
 */
class DirectionCrossCollection: public std::vector<DirectionCross> {
public:
    std::string name = "not set";
    vector<Direction*> underlyingDirections;
    void setName(const std::string &name) {DirectionCrossCollection::name = name;}
    DirectionCrossCollection() = default;
    void merge(const DirectionCrossCollection& collection) {
        for (const DirectionCross& bc : collection) {
            this->push_back(bc);
        }
    }
    Vector2d getPositionFirstGuess();
    Vector3d getMeanCrossLocation(int exclusionIndex);
    double findOutliers(Direction* direction);
    void analyse();
    void list();
    std::string toString();
    std::string toJSON();
    friend std::ostream& operator<<(std::ostream &strm, const DirectionCrossCollection &directionCrossCollection);

    unordered_map<int, int> parent;

    // perform MakeSet operation
    void makeSet(vector<int> const &universe)
    {
        // create `n` disjoint sets (one for each item)
        for (int i: universe) {
            parent[i] = i;
        }
    }

    int Find(int k)
    {
        // if `k` is root
        if (parent[k] == k) {
            return k;
        }

        // recur for the parent until we find the root
        return Find(parent[k]);
    }

    // Perform Union of two subsets
    void Union(int a, int b)
    {
        // find the root of the sets in which elements `x` and `y` belongs
        int x = Find(a);
        int y = Find(b);

        parent[x] = y;
    }

    map<int, DirectionCollection> findDisjointSets() {

        vector<int> universe(underlyingDirections.size());
        std::iota(universe.begin(), universe.end(), 0);
        this->makeSet(universe);

        for (const auto& cross : *this) {
            if(cross.included) {
                cout << cross.direction1->id << " x " << cross.direction2->id << endl;
                this->Union(cross.direction1->id, cross.direction2->id);        // 4 and 3 are in the same set
            }
        }

        map<int, DirectionCollection> disjointSets;
        for (int i : universe) {
            int groupId = this->Find(i);
            if (disjointSets.count(groupId) == 0) {
                DirectionCollection newGroup("group_" + to_string(groupId));
                disjointSets[groupId] = newGroup;
            }
            disjointSets[groupId].insert(underlyingDirections.at(i));
            cout << i << " "<< groupId << " " << endl;
        }

        return disjointSets;
    }

private:
    static VectorXd getPositionGuess(int nLocations, MatrixXd crossCoordinates);


};

#endif //WEBASSEMBLY_DIRECTIONCROSSCOLLECTION_H
