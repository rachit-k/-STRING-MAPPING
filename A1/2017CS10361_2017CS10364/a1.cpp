#include <iostream>
#include <unordered_map> 
#include <string>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
//#include <boost/algorithm/string.hpp>
#include <algorithm>
#include <random>
#include <fstream>
#include <chrono>       // std::chrono::system_clock


using namespace std;


int modv;       //total size of vocab
vector<string> vocabulary;      //vocab array
vector<string> string_list;     //input string lists
vector<vector<int> > converted_string_list;     //converted strings to numbers e.x A - 0; C - 1; T - 2; G - 3; dash - 4
unordered_map<string, int> vocab_map;   //map of vocab to numbers
int k;  //number of strings
vector<vector<int> > mc;    //cost matrix


int cost_new(vector<vector<int> > counts,int row,int col1,int col2)
{
	int cost = 0;
	for(int i=0;i<=counts[0].size();i++)
	{
		if(i==row)
			continue;
		cost=cost+mc[row][i]*counts[col1][i]*counts[col1][row]+mc[row][i]*counts[col2][i]*counts[col2][row];
	}
    return cost;
}

int changed_cost(vector<vector<int> > counts,int row,int col1,int col2)
{
    int c1 = 0;
    int c2 = 0;
    for(int ii = 0;ii<counts[0].size();ii++)
        c1 = c1 + (mc[ii][col1] - mc[ii][col2])*counts[row][ii];
    c1 = c1 + mc[col1][col2];
    
    for(int ii = 0;ii<counts[0].size();ii++)
        c2 = c2 + (mc[ii][col2] - mc[ii][col1])*counts[row+1][ii];
    c2 = c2 + mc[col1][col2];

    return -(c1+c2);
}

//cost computation   
int cost(vector<vector<int> > mc, vector<vector<int> > counts)
{
    int cost = 0;
    for(int i =0;i<counts.size();i++)
    {
        for(int j = 0;j<counts[0].size();j++)
        {
            for(int ii = j+1;ii<counts[0].size();ii++){
                cost = cost + mc[ii][j]*counts[i][j]*counts[i][ii];
            }
        }
    }
    return cost;       
}

class State 
{ 
    public: 
  
    // Data Members 
    int currentCost;
    vector<vector<int> > string_array;
    int maxlength;  //common length of the current state
    vector<vector<int> > counts;    //array that holds number of each letter at the i'th position
    unordered_map<int, vector<int> > possible_switches; //for each row, what are the places where I can do a right switch
    

    //create Initial State with padded dashes at the right
    void createInitialState(int fromScratch) 
    { 
        if(fromScratch == -1){
            maxlength = 0;
            for(int i = 0;i<converted_string_list.size();i++)
                if(maxlength <converted_string_list[i].size())
                    maxlength = converted_string_list[i].size();
        }
        else
            maxlength = fromScratch;
        
            
        vector<int> letters(modv+1,0);
        letters[modv] = k;
        for(int j = 0;j<maxlength;j++)
        {
            counts.push_back(letters);
        }
        for(int i = 0;i<k;i++)
        {
            vector<int> s;
            possible_switches[i] = s;
        }

        for(int i = 0;i<converted_string_list.size();i++)
        {
            vector<int> v(maxlength, modv);
            int j =0;
            for(;j<converted_string_list[i].size();j++){
                v[j] = converted_string_list[i][j];
                counts[j][converted_string_list[i][j]]++;
                counts[j][modv]--;
            }
            if(j<maxlength-1)
                possible_switches[i].push_back(j-1);
            //for(j=j-1;j<maxlength-1;j++)
            //{
            //    possible_switches[i].push_back(j);
            //}
            string_array.push_back(v);
        }
        currentCost = cost(mc,counts);     
    }

    //Move to the Minimum Neighbour. However if random parameter is true, it moves the dashes. This is for initialisation of dashes to middle positions
    bool MinRandomNeighbour(bool random)
    {
        int mincost = 0;
        int minrow = -1;
        int minswitcher = -1;
        if(random){
            for(int i = 0;i<k;i++)
            {
                vector<int> rowchanges = possible_switches[i];
                //set <int> :: iterator itr;
                vector<int> s;
                
                    
                for (int itr = 0; itr < rowchanges.size(); itr++) 
                {
                    int iSecret = rand() % 3;
                    
                    if(iSecret==0){
                        int switcher = rowchanges[itr];
                        if(string_array[i][switcher] == modv && string_array[i][switcher+1] == modv)
                            continue;
                        //cout << i << " " << 2 << "\n";
                        counts[switcher][string_array[i][switcher]]--;
                        counts[switcher+1][string_array[i][switcher+1]]--;
                        counts[switcher][string_array[i][switcher+1]]++;
                        counts[switcher+1][string_array[i][switcher]]++;
                        int temp = string_array[i][switcher];
                        string_array[i][switcher] = string_array[i][switcher+1];
                        string_array[i][switcher+1] = temp;
                        break;
                    }
                }
                for(int j = 0;j<maxlength-1;j++)
                {
                    if(j==0 && string_array[i][j] == modv)
                    {
                        if(maxlength>1)
                            if(string_array[i][j+1]!=modv)
                                s.push_back(j);
                    }
                    else if(string_array[i][j+1] == modv){
                        if(string_array[i][j]!=modv)
                            s.push_back(j);
                    }
                    else if(string_array[i][j] == modv){
                        if(string_array[i][j+1]!=modv)
                            s.push_back(j);
                    }
                }
                //cout << "\n";
                possible_switches[i] = s;
            }
            return false;
        }


        else{
            for(int i = k-1;i>-1;i--)
            {
                vector<int> rowchanges = possible_switches[i];
                //cout << rowchanges.size() << "\n";
                //set <int> :: iterator itr;
                //seed = std::chrono::system_clock::now().time_since_epoch().count();
                //auto gen = std::default_random_engine(seed);
                //shuffle (rowchanges.begin(), rowchanges.end(), gen);
                //shuffle(rowchanges.begin(), rowchanges.end(), default_random_engine(seed));
                for (int itr = 0; itr <rowchanges.size(); itr++) 
                { 
                    int switcher = rowchanges[itr];
                    if(string_array[i][switcher] == modv && string_array[i][switcher+1] == modv)
                        continue;
                    // counts[switcher][string_array[i][switcher]]--;
                    // counts[switcher+1][string_array[i][switcher+1]]--;
                    // counts[switcher][string_array[i][switcher+1]]++;
                    // counts[switcher+1][string_array[i][switcher]]++;
                    // int co = cost(mc, counts);
                    
                    // //cost_new(vector<vector<int> > counts,int row,int col1,int col2)
                    // //cost_new(counts,int row,int col1,int col2)
                    // //cout << co << "\n";
                    // counts[switcher][string_array[i][switcher]]++;
                    // counts[switcher+1][string_array[i][switcher+1]]++;
                    // counts[switcher][string_array[i][switcher+1]]--;
                    // counts[switcher+1][string_array[i][switcher]]--;
                    int co = changed_cost(counts, switcher,string_array[i][switcher],string_array[i][switcher+1]);
                    
                    if(co <= mincost)
                    {
                        mincost = co;
                        minrow = i;
                        minswitcher = switcher;
                    }


                }
            }
            if(minrow != -1)
            {
                counts[minswitcher][string_array[minrow][minswitcher]]--;
                counts[minswitcher+1][string_array[minrow][minswitcher+1]]--;
                counts[minswitcher][string_array[minrow][minswitcher+1]]++;
                counts[minswitcher+1][string_array[minrow][minswitcher]]++;
                int temp = string_array[minrow][minswitcher];
                string_array[minrow][minswitcher] = string_array[minrow][minswitcher+1];
                string_array[minrow][minswitcher+1] = temp;
                vector<int> s;
                for(int j = 0;j<maxlength-1;j++)
                {
                    if(j==0 && string_array[minrow][j] == modv)
                    {
                        if(maxlength >1)
                            if(string_array[minrow][j+1] !=modv)
                                s.push_back(j);
                    }
                    else if(string_array[minrow][j+1] == modv){
                        if(string_array[minrow][j] != modv)
                            s.push_back(j);
                    }
                    else if(string_array[minrow][j] == modv){
                        if(string_array[minrow][j+1] != modv)
                            s.push_back(j);
                    }
                }
                possible_switches[minrow] = s;
                //cout << "WHY AM I NOT HERE: "<<currentCost <<" " <<mincost<<"\n";
                currentCost = currentCost + mincost;
                //cout << minrow << " " << minswitcher << "\n";
                return true;
                
            }
            else return false;
        }
    } 

    //get cost of the state;
    int state_cost()
    {
        return currentCost;
    }

    //make a random switch
    void randomJump()
    {
        int i = rand()%(string_array.size());
        random_shuffle(possible_switches[i].begin(),possible_switches[i].end());
        if(possible_switches[i].size() == 0)
            return;
        int j = rand()%(possible_switches[i].size());
        j = possible_switches[i][j];

        
        //cout << i << " " << j <<"\n";
        //cout << "Random :" << cost(mc,counts) << " ";
        //currentCost = currentCost + changed_cost(counts,j,string_array[i][j],string_array[i][j+1]);
        //cout <<   << " ";
        currentCost = currentCost + changed_cost(counts,j,string_array[i][j],string_array[i][j+1]);
        counts[j][string_array[i][j]]--;
        counts[j+1][string_array[i][j+1]]--;
        counts[j][string_array[i][j+1]]++;
        counts[j+1][string_array[i][j]]++;
        int temp = string_array[i][j];
        string_array[i][j] = string_array[i][j+1];
        string_array[i][j+1] = temp;
        vector<int> s;
        for(int j = 0;j<maxlength-1;j++)
        {
            if(j==0 && string_array[i][j] == modv)
            {
                if(maxlength>1)
                    if(string_array[i][j+1] != modv)
                        s.push_back(j);
                //cout <<j;
            }
            else if(string_array[i][j+1] == modv){
                if(string_array[i][j] != modv)
                    s.push_back(j);
                //cout <<j;
            }
            else if(string_array[i][j] == modv){
                if(string_array[i][j+1] != modv)
                    s.push_back(j);
                //cout <<j;
            }
        }
        
        possible_switches[i] = s;
        //cout << cost(mc,counts)<<"\n";
    }
}; 

//convert string to number format, using the vocab mapping
vector<int> convertStringWithVocab(string s)
{
    vector<int> v;
    for(int i = 0;i<s.length();i++)
        v.push_back(vocab_map[s.substr(i,1)]);
    return v;
}

//trimming whitespace for input
string trim(const string& str)
{
    size_t first = str.find_first_not_of(' ');
    if (string::npos == first)
    {
        return str;
    }
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}


int main(int argc, char *argv[])
{
    std::chrono::time_point<std::chrono::system_clock> begin = std::chrono::system_clock::now();
    ifstream input_data;
    input_data.open(std::string(argv[1]));

    ofstream output_data;
    output_data.open(std::string(argv[2]));

    srand((unsigned)time(0));
    float time;
     
    input_data >> time;
    input_data >> modv;
    string token;
    // 
    for(int i = 0;i<modv-1;i++)
    {
        input_data >> token;
        //boost::erase_all(token, ",");
        std::replace( token.begin(), token.end(), ',', ' ');
        token = trim(token);
        vocabulary.push_back(token);
    }
    input_data >> token;
    vocabulary.push_back(token);
    for(int i = 0;i<vocabulary.size();i++)
    {
        vocab_map[vocabulary[i]] =  i; 
    }
    vocab_map["-"] = vocabulary.size();
    
    input_data >> k;
    int maxlength = 0;
    for(int i = 0;i<k;i++){
        input_data >> token;
        if(token.size() > maxlength)
            maxlength = token.size();
        string_list.push_back(token);
        converted_string_list.push_back(convertStringWithVocab(token));
    }
    
    int cc;
    input_data >> cc;

    int basecost = 0;
    for(int i = 0;i<k;i++)
    {
        basecost = basecost + (maxlength - string_list[i].size());
    }
    basecost = basecost*cc;
    vector<int> mcc(modv+1,0);
    for(int i = 0;i<modv+1;i++)
        mc.push_back(mcc);

    for(int i = 0;i<modv+1;i++)
        for(int j = 0;j<modv+1;j++)
            input_data >> mc[i][j];
    int mincost = 1000000000;
    State beststate;
    int ofMinCost = 0;
    input_data >> token;
    vector<int> best;
    bool updated = true;
    float timeleft,slot;
    auto ms = (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - begin)).count();
    
    vector<int> check;
    check.push_back(maxlength);
    check.push_back(maxlength+1);
    check.push_back(maxlength+2);
    check.push_back(maxlength+3);
    check.push_back(maxlength+4);
    check.push_back(maxlength+5);
    if(maxlength < 15){
    check.push_back(maxlength+7);
    check.push_back(maxlength+9);
    check.push_back(maxlength+12);
    check.push_back(maxlength+14);
    }
    else{
        check.push_back(maxlength+(maxlength/2));
    check.push_back(maxlength+((4*maxlength)/5));
    check.push_back(maxlength+((3*maxlength)/4));
    check.push_back(maxlength+maxlength);
    }
    bool higher = false;
    //check.push_back(maxlength*maxlength);
    for(int alpha = 0;alpha<check.size();alpha++)
    {
        int size = check[alpha];
        updated = true;
        //cout << "Shallow: " << size<<"\n";
        for(int randomrestart = 0;randomrestart<1;randomrestart++)
        {
            if(cc>10 && alpha >5)
                break;
            //cout << size << "\n";
            State state;
            state.createInitialState(size);
            for(int i = 0;i<5000;i++)
            {
                ms = (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - begin)).count();
                if((float)ms/60000 > (float)(45*time)/60)
                    goto label;
                bool ans = state.MinRandomNeighbour(true);
            }
            std::chrono::time_point<std::chrono::system_clock> current = std::chrono::system_clock::now();
            //auto difference = std::chrono::duration_cast<std::chrono::milliseconds>(current- begin);
            //auto ms = difference.count();
            //cout << (float)ms/60000 <<"\n";
            //cout << "Time :"<< time <<"\n";
            
            state.currentCost = cost(mc,state.counts);
            
            for(int i = 0;i<10000;i++){
                ms = (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - begin)).count();
                if((float)ms/60000 > (float)(45*time)/60)
                    goto label;
                int which;
                if(i<2500)
                    which = rand() % 3;
                else
                    which = rand() % 5;
                bool ans = true;
                if(which == 0)
                    state.randomJump();
                else
                    ans = state.MinRandomNeighbour(false);
                
                
                int cosss = state.state_cost();
                //cout << cosss << " " << cost(mc,state.counts) <<"\n";
                if(cosss + (cc*k*(size-maxlength)) < mincost){
                    //cout <<"Here";
                    if(alpha > 5)
                        higher = true;
                    mincost = cosss +(cc*k*(size-maxlength));
                    beststate = state;
                    ofMinCost = i;
                    //cout << "Cost " << mincost << " At " << i <<"\n";
                    if(updated == true)
                    {
                        updated = false;
                        if(best.size()>=1)
                        {
                            //cout << "Last was: " << best[best.size()-1]<<"\n";
                            if(best[best.size()-1] == size-1){
                                best[best.size()-1]++;
                                //cout <<"Not Added " <<best[best.size()-1] << "\n"; 
                            }
                            else{
                                best.push_back(size);
                                //cout << "Added "<< best[best.size()-1] <<"\n";
                            }
                        }
                        else{
                                best.push_back(size);
                                //cout << "Added "<< best[best.size()-1] <<"\n";
                            }
                    }
                }
            }
        }
    }
    for(int i = 0;i<beststate.string_array.size();i++)
    {
        for(int j = 0;j<beststate.string_array[i].size();j++){
            int x = beststate.string_array[i][j];
            if(x==modv)
                output_data << "-";
            else
                output_data << vocabulary[x];
        }
        output_data << "\n";
    }
    if(higher)
    {
        if(best.size()!=0)
        {
            vector<int> v;
            int a = best[best.size()-1];
            if(a-2>=maxlength)
                v.push_back(a-2);
            v.push_back(a);
            v.push_back(a+2);
            best = v;
        }
    }
    else if(best.size()==1){
        if(best[best.size()-1] >maxlength)
            best.push_back(best[best.size()-1]-1);
        best.push_back(best[best.size()-1]+1);
    }
    
    
    //cout << "higher" << " " << best.size() << "\n";
    //cout <<"HEY: "<<best.size();  
    ms = (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - begin)).count();
    //if((float)ms/60000 > (float)(45*time
    cout << "TimeLeft: " << (float)ms/60000 << "\n";
    timeleft = (float)(time) - ((float)ms)/60000;
    //cout << "Time: " <<timeleft <<"\n";
    slot = timeleft/best.size();
    //cout << "Slot: " << slot << "\n";
    for(int size = 0;size<best.size();size++)
    {
        std::chrono::time_point<std::chrono::system_clock> beginslot = std::chrono::system_clock::now();
        
        auto timeofthisslot = 0;
        while((float)timeofthisslot/60000 < (50*slot)/60)
        {
            ms = (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - begin)).count();
            
            //cout << size << "\n";
            State state;
            
            state.createInitialState(best[size]);
            for(int i = 0;i<5000;i++)
            {
                if(i%100 == 0)
                {
                    ms = (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - begin)).count();
                    if((float)ms/60000 > (float)(55*time)/60)
                        goto label;
                }
                bool ans = state.MinRandomNeighbour(true);
            }
            
            state.currentCost = cost(mc,state.counts);
            
            for(int i = 0;i<50000;i++){
                
                if(i%100 == 0)
                {
                ms = (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - begin)).count();
                if((float)ms/60000 > (float)(55*time)/60)
                    goto label;
                }
                
                int which;
                if(i<20000)
                    which = rand() % 3;
                else
                    which = rand() % 5;
                bool ans = true;
                if(which == 0)
                    state.randomJump();
                else
                    ans = state.MinRandomNeighbour(false);
                
                
                int cosss = state.state_cost();
                //cout << cosss << " " << cost(mc,state.counts) <<"\n";
                if(cosss + (cc*k*(best[size]-maxlength)) < mincost){
                    mincost = cosss +(cc*k*(best[size]-maxlength));
                    beststate = state;
                    ofMinCost = i;
                    //cout << "Cost " << mincost << " At "  << cost(mc,beststate.counts)<< " "<< i <<"\n";
                }
                timeofthisslot = (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - beginslot)).count();             
            }
        }
    }
    

output_data.close();
output_data.open(std::string(argv[2]), std::ofstream::out | std::ofstream::trunc);

label:
    for(int i = 0;i<beststate.string_array.size();i++)
    {
        for(int j = 0;j<beststate.string_array[i].size();j++){
            int x = beststate.string_array[i][j];
            if(x==modv)
                output_data << "-";
            else
                output_data << vocabulary[x];
        }
        output_data << "\n";
    }
    output_data.close();
    ms = (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - begin)).count();
            
    //cout << (float)ms/60000 <<"\n";
    return 0;
}