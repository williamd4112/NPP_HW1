#include<iostream>
#include<set>
#include<cstring>
#define MAX 30005
using namespace std;

int cases, pairs , citizens, a, b, friend_count, max_friend;
bool friends[MAX];
set<int> graph[MAX];

void build_graph() {
    for(int i = 0; i < pairs ;i++) {
        cin >> a >> b;
        graph[a].insert(b), graph[b].insert(a);
    }
}

void DFS(set<int> node) {
    for(std::set<int>::iterator iter = node.begin(); iter != node.end() ;iter++) {
        if(friends[*iter] == false) { //didn't visit yet
            friends[*iter] = true; //set as visit
            friend_count++;             //one more friend
            DFS(graph[*iter]);
        }
    }
}

int main(){
    while(cin >> cases) {
        while(cases--) {
            cin >> citizens >> pairs;
            build_graph();
            for(int i = 1; i <= citizens ;i++) {
                if(friends[i] == 0) {
                    friend_count = max_friend = 0;
                    DFS(graph[i]);
                    if(max_friend < friend_count) max_friend = friend_count;
                }
            }
            cout << max_friend << endl;
        }
    }
    return 0;
}
