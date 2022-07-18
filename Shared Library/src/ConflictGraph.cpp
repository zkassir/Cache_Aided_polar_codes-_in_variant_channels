#include "ConflictGraph.h"

namespace caching{

/* Variables declaration */
int n_utenti;
vector<int> chunks_Node; //Nb chunks per packet/node per user
int m_files;
int b_chunks;
vector<int> nodi_user; //Nb nodes per user
int m_archi;
unsigned int sizenodes;
unsigned int size_node_list;
int id_last;
int ***Ind = NULL;
int *Q = NULL;
int **Q_chuncks = NULL;

vector<nodo> nodes; //The vector (structure) of nodes
int **Matrix_Adj = NULL;
nodo *node_list = NULL;
nodo *node_listc = NULL;
vector<nodo> node_list_updated;
vector<nodo> node_list_unchanged;
int **Matrix_Adj2 = NULL;
int *newColors;

/*New variables declaration*/
vector<vector<nodo>> user_node;
vector<int> chunks_Node2;	//Nb sub-chunks in node with new code-rate

/*************************************************************************************************************/
void computeNumberOfNodes(){
    int i, k, id_file;

    cout << "--- Compute Nodes ---" << endl;
    /*For each user ...*/
    for (i=0; i<n_utenti; i++){
        //nodi_user[i] will contain at the end the number of chunks requested by (not in the cache of) user i 
        nodi_user.push_back(0);
        /*The file that the user 'i' request*/
        id_file = Q[i];
        /*For each chunk ...*/
        for (k=0; k<b_chunks; k++){
            
            /*Check if user 'i' have in its cache the chunk 'k' related to files 'id_file'*/
            if (Ind[i][id_file][k] == 1){
                Q_chuncks[i][k] = 0;
            }else{
                Q_chuncks[i][k] = 1;
                nodi_user[i]++;
            }
            //nodi_user[i] = ceil(nodi_user[i]/chunks_Node[i]);
        }
        cout << nodi_user[i] << ", ";
    }
    cout << endl;
}

/*This is a function that builds the nodes of the conflict information graph*/
void makeNodes(){
    int i, j, k, l, id, nb_pck;
    id = 0; nb_pck = 0;
    nodo n1;

    cout << "--- Make Nodes ---" << endl;

    /*For each user and for each chunk related to the requested file 
    that the user do not have in cache will be created a node*/
    for (i=0; i<n_utenti; i++){
        //nb_pck will contain the number of packets required given the code rate (chunks_Node)
        nb_pck += ceil((double)nodi_user[i]/chunks_Node[i]);
        cout<< "Number of Packets of user "<< i <<" is "<< nb_pck << ", ";
        vector<unsigned int> chunks_f;
        j = Q[i]; l  = 0; k=0;
        //cout << chunks_Node[i] << endl;
        if(chunks_Node[i]==3){           
            //Add the remaining packets to the vector of packets
            for (int k = 0; k < b_chunks; k++){
                if (Q_chuncks[i][k] == 1){ //index == -1 && 
                    if(l==0){
                        n1.id = id;
                        n1.degree = 0;
                        n1.id_utente = i;
                        n1.id_file = j;
                        n1.id_chunck.push_back(k);
                        chunks_f.push_back(k);
                        l++;
                    }else if(l==1){
                       n1.id_chunck.push_back(k);
                       l++;
                    } else if (l==2){
                        n1.id_chunck.push_back(k);
                        nodes.push_back(n1);
                        n1.id_chunck = vector<int> ();
                        l=0;
                        id++; 
                    }   
                } 

                if((l==1 || l==2) && (k == (b_chunks-1))) {
                    nodes.push_back(n1);
                    n1.id_chunck = vector<int> ();
                    id++;
                }
            }
            /*for(int cc=0; cc< nodes.size(); cc++)
                for(int ccc=0; ccc<nodes[cc].id_chunck.size(); ccc++)
                    cout << nodes[cc].id_chunck[ccc] << ", ";*/

        } else if (chunks_Node[i] < 3){
            while(k < b_chunks){
                if(Q_chuncks[i][k] == 1){
                    if(l==0 || chunks_Node[i] == 1){
                        n1.id = id;
                        n1.degree = 0;
                        n1.id_utente = i;
                        n1.id_file = j;
                        n1.id_chunck.push_back(k);
                        l++;
                        if(l == chunks_Node[i] || k == (b_chunks-1)){//here l==chunks_Node[i] means chunks_Node[i] =1
                            nodes.push_back(n1);
                            n1.id_chunck = vector<int> ();
                            l=0;
                            id++;
                            //cout << k << ", " << id << endl;
                        }
                    }else if (l < chunks_Node[i]){
                        n1.id_chunck.push_back(k);
                        l++;
                        if(l == chunks_Node[i] || k == (b_chunks-1)){
                            nodes.push_back(n1);
                            n1.id_chunck = vector<int> ();
                            l=0;
                            id++;
                        }
                    }
                }      
                k++;   
            }
        } else {
            cout << "Case NOT Suported!" << endl;
            exit(0);
        }
        cout << "\nNb of nodes: " << nodes.size() << endl;
        
        if (id != nb_pck){
            cout << "\nError: Create Nodes Number Dismatch With The Aspected Nodes Number.\n";
            cout << id << "!=" << nb_pck << endl;
            exit(0);
        }
    }
    //id_last =id;
    sizenodes = nodes.size();
    cout << "Number of node: " << sizenodes << endl;
    cout << "--- Make Nodes Completed ---" << endl;
    
}

/*This is a function that provide to make a edges of the conflict information graph*/
void makeEdges(){
    int i_1, j_1, i_2, j_2, id1, id2;
    unsigned int i, j, cc;
    cc=0;
    
    vector<int> c1;
    vector<int> c2;
    bool edge;
    m_archi = 0;

    cout << "--- Make Edges ---" << endl;

    Matrix_Adj = (int **) malloc (sizenodes * sizeof(int *));
    check_memory_allocation_2D(Matrix_Adj, "Allocation 2D Adj Matrix.");

    for (j=0; j<sizenodes; j++){
        Matrix_Adj[j] = (int *) malloc (sizenodes * sizeof(int));
        check_memory_allocation_1D(Matrix_Adj[j], "Allocation 1D Adj Matrix.");
    }

    for (j=0; j<sizenodes; j++){
        Matrix_Adj[j][j] = 0;
    }

    for (id1 = 0; id1 < sizenodes-1; ++id1){
        for (id2 = (id1+1); id2 < sizenodes; ++id2){
            i_1 = nodes[id1].id_utente;
            j_1 = nodes[id1].id_file;
            c1 = nodes[id1].id_chunck;

            i_2 = nodes[id2].id_utente;
            j_2 = nodes[id2].id_file;
            c2 = nodes[id2].id_chunck;

            //Check if all chunks associated to node id1 are in the cache of the user associated with node id2
            edge = false;
            for(i=0; i<c1.size(); i++){
                if(Ind[i_2][j_1][c1[i]] == 0 )
                    edge = true;
            }
            for(i=0; i<c2.size(); i++){
                if(Ind[i_1][j_2][c2[i]] == 0 )
                    edge = true;
            }
            if(edge){
                Matrix_Adj[id1][id2] = 1;
                Matrix_Adj[id2][id1] = 1;
                nodes[id1].degree++;
                nodes[id2].degree++;
                m_archi += 2;
            } else{
                Matrix_Adj[id1][id2] = 0;
                Matrix_Adj[id2][id1] = 0;
                cc++;
            }
            
        }
    }
    //cout << cc << endl;
}

/****************************************************************************************************************/
void makeEdges_ON_node_list(){
    int i_1, j_1, i_2, j_2, id1, id2;
    unsigned int i, j, cc;
    cc=0;
    
    vector<int> c1;
    vector<int> c2;
    bool edge;
    m_archi = 0;
    cout <<endl<< "--- Make Edges node_list_updated ---" << endl;

    Matrix_Adj2 = (int **) malloc (size_node_list * sizeof(int *));
    check_memory_allocation_2D(Matrix_Adj2, "Allocation 2D Adj Matrix.");

    for (j=0; j<size_node_list; j++){
        Matrix_Adj2[j] = (int *) malloc (size_node_list * sizeof(int));
        check_memory_allocation_1D(Matrix_Adj2[j], "Allocation 1D Adj Matrix.");
    }

    for (j=0; j<size_node_list; j++){
        Matrix_Adj2[j][j] = 0;
    }

    for (id1 = 0; id1 < size_node_list-1; ++id1){
        for (id2 = (id1+1); id2 < size_node_list; ++id2){
            i_1 = node_list_updated[id1].id_utente;
            j_1 = node_list_updated[id1].id_file;
            c1 = node_list_updated[id1].id_chunck;

            i_2 = node_list_updated[id2].id_utente;
            j_2 = node_list_updated[id2].id_file;
            c2 = node_list_updated[id2].id_chunck;

            //Check if all chunks associated to node id1 are in the cache of the user associated with node id2
            edge = false;
            for(i=0; i<c1.size(); i++){
                if(Ind[i_2][j_1][c1[i]] == 0 )
                    edge = true;
            }
            for(i=0; i<c2.size(); i++){
                if(Ind[i_1][j_2][c2[i]] == 0 )
                    edge = true;
            }
            if(edge){
                Matrix_Adj2[id1][id2] = 1;
                Matrix_Adj2[id2][id1] = 1;
                node_list_updated[id1].degree++;
                node_list_updated[id2].degree++;
                m_archi += 2;
            } else{
                Matrix_Adj2[id1][id2] = 0;
                Matrix_Adj2[id2][id1] = 0;
                cc++;
            }
            
        }
    }
    //cout << cc << endl;
}

/****************************************************************************************************************/
void _dealloc(){
    int i;

    free(Q);
    Q = NULL;

    for (i=0; i<n_utenti; i++){
        free(Q_chuncks[i]);
        Q_chuncks[i] = NULL;
    }

    free(Q_chuncks);
    Q_chuncks = NULL;

    nodes = vector<nodo> ();

}

/*****************************************************************************************************************/
/*NEW CODE*/

void splitNode(int userID,int z)	//user's id ,index of user in the user_node vector, id of last node in Graph
{
	vector<int> C;	//will contain sub-chunks overflowing from nodes
	int nb_pck =0, nb_pck2=0 , epsilon, extra_Nodes;
	nodo n1;
	int file, l=0;
	int id =id_last +1;
	nb_pck += ceil((double)nodi_user[userID]/chunks_Node[userID]);   //nbr of nodes wrt old rate
	nb_pck2 += ceil((double)nodi_user[userID]/chunks_Node2[userID]); //nbr of nodes wrt new rate
	epsilon= chunks_Node[userID] - chunks_Node2[userID];		 //nbr of sub-chunks to eliminate from each node
	extra_Nodes= nb_pck2 - nb_pck;					 //nbr of nodes to be added

	for(int i = 0; i < user_node[z].size(); ++i){
		//put extra sub-chunks of nodes related to user in C
        if(user_node[z][i].id_chunck.size()>chunks_Node2[userID]){
		  for (int j=0; j<epsilon ; j++){
                if(user_node[z][i].id_chunck.size()==1) break;
                C.push_back(user_node[z][i].id_chunck.back()); /**************check**************/
                user_node[z][i].id_chunck.pop_back();       //delete the taken subchunks from nodes
            }
        }

        if(i==user_node[z].size()-1) id= user_node[z][i].id +1;
	}

    //when done sort C as to have sub-chunks be consequetive
	sort(C.begin(), C.end());

    /*cout<<"\n C= ";
    for(int i=0;i<C.size();i++)
        cout<<C[i]<<" ";
    cout<<endl<<endl;*/

    //special case
    if(chunks_Node[userID]==3 && chunks_Node2[userID]==2 && user_node[z].back().id_chunck.size()==1){
        user_node[z].back().id_chunck.push_back(C.back());
        C.pop_back();
    }

	file = Q[userID]; //file ID

	//start creating nodes and add to user_node
    int k=0; 
    while(k < C.size()){
        if(l==0 || chunks_Node2[userID] == 1){
            n1.id = id;
            n1.degree = 0;
            n1.id_utente = userID;
            n1.id_file = file;
            n1.id_chunck.push_back(C[k]);
            n1.uncolored=true;
            n1.col=0;
            l++;
            if(l == chunks_Node2[userID] || k == (C.size()-1)){
            	user_node[z].push_back(n1);
            	n1.id_chunck = vector<int> ();
                l=0;
            	id++;
            }
        }else if (l < chunks_Node2[userID]){
            n1.id_chunck.push_back(C[k]);
            l++;
            if(l == chunks_Node2[userID] || k == (C.size()-1)){
                user_node[z].push_back(n1);
                n1.id_chunck = vector<int> ();
                l=0;
                id++;
            }
        }
        k++;   
    }

    id_last=id;
    if(user_node[z].size()!= nb_pck2) cout<<"\n ERROR ADDING  NODES --- "<<"size= "<<user_node[z].size()<<" nb_pck2= "<<nb_pck2<<endl;;
    for(int i=userID+1;i<n_utenti;i++){
        if(chunks_Node[i]==chunks_Node2[i]){
            for(int j=0; j<node_list_unchanged.size();j++){
                if(node_list_unchanged[j].id_utente==i)
                    node_list_unchanged[j].id+=extra_Nodes;
            }
        }
        else{
            for(int j=0; j<user_node.size();j++){
                for(int k=0;k<user_node[j].size();k++)
                if(user_node[j][k].id_utente==i)
                    user_node[j][k].id+=extra_Nodes;
            }
        }
    }
    
    /*for(int i=0;i<user_node[z].size();i++){
        for(int j=0; j<chunks_Node2[userID];j++)
        cout<<"ID= "<<user_node[z][i].id<<" chunk= "<<user_node[z][i].id_chunck[j]<<" ";
        cout<<endl;
    }*/


}
/****************************************************************************************************************/
/*NEW CODE*/
void MergeNode(int userID, int z) //user's id ,index of user in the user_node vector
{
	vector<int> C;	//will contain sub-chunks of additional nodes that will be deleted from graph
    vector<int> nodeID; //will contain nodes' IDs i should delete;
	int nb_pck =0, nb_pck2=0 , epsilon, extra_Nodes, counter=0;
	int l=0;
    vector<int> pointer = {3,2,2};

	nb_pck += ceil((double)nodi_user[userID]/chunks_Node[userID]);   //nbr of nodes wrt old rate
	nb_pck2 += ceil((double)nodi_user[userID]/chunks_Node2[userID]); //nbr of nodes wrt new rate
	epsilon= chunks_Node2[userID] - chunks_Node[userID];		 //nbr of sub-chunks to add to nodes
	extra_Nodes= nb_pck - nb_pck2;					 //nbr of nodes to be deleted

	//fill up C vector according to code-rate change
	for(int i=0;i<user_node[z].size();i++){
		//case1: 1/6 to 1/3
		if(chunks_Node[userID]==1 && chunks_Node2[userID] == 2){
			if(i>=pointer[0]){ C.push_back(user_node[z][i].id_chunck[0]); counter++; nodeID.push_back(user_node[z][i].id); }
			if(counter==3 ){ pointer[0]=pointer[0]+6; counter=0; }
            if((user_node[z].size()%6 ==2 || user_node[z].size()%6 == 3 ) && i==user_node[z].size()-1) { C.push_back(user_node[z][i].id_chunck[0]); nodeID.push_back(user_node[z][i].id); } 
            if(user_node[z].size()%6 == 4 && i==user_node[z].size()-4) pointer[0]-=1;   
		}
		//case2: 1/6 to 1/2
		if(chunks_Node[userID]==1 && chunks_Node2[userID] == 3){
			if(i>=pointer[1]){ C.push_back(user_node[z][i].id_chunck[0]); counter++; nodeID.push_back(user_node[z][i].id); }
			if(counter==4){ pointer[1]=pointer[1]+6; counter=0; }
            if(user_node[z].size()%6 ==2 && i==user_node[z].size()-1) { C.push_back(user_node[z][i].id_chunck[0]); nodeID.push_back(user_node[z][i].id); } 
            if(user_node[z].size()%6 ==3 && i==user_node[z].size()-3)   pointer[1]-=1;
		}
		//case3: 1/3 to 1/2
		if(chunks_Node[userID]==2 && chunks_Node2[userID] == 3){
			if(i==pointer[2]){ 
                for (int k=0; k< chunks_Node[userID]; k++){
						C.push_back(user_node[z][i].id_chunck[k]);
                        if(user_node[z][i].id_chunck.size()==1) break;
				}
				counter++;
                //if(user_node[z][i].id_chunck.size()!=1) 
                    nodeID.push_back(user_node[z][i].id);
			}
			if(counter==1){ pointer[2]=pointer[2]+3; counter=0;}
            if (user_node[z].size()%3 ==2 && user_node[z][i].id_chunck.size()==1) {C.push_back(user_node[z][i].id_chunck[0]);  nodeID.push_back(user_node[z][i].id); }

		}
	}
    //cout<<endl<<"C vector done"<<endl;
	sort(C.begin(), C.end());

    /*cout<<"\n C= ";
    for(int i=0;i<C.size();i++)
        cout<<C[i]<<" ";
    cout<<endl;*/

	//delete nodes that we extracted sub-chunks from
	for(int i=0;i<nodeID.size();i++){
		for(int j=0; j<user_node[z].size();j++){
            if(user_node[z][j].id == nodeID[i]){
                user_node[z].erase(user_node[z].begin()+j);
                break;
            }
        }
	}
    //cout<<endl<<"delete node done"<<endl;

	if(user_node[z].size() != nb_pck2){
        cout<<endl<<"****ERROR with delete****; size= "<<user_node[z].size()<<" nb_pck2= "<<nb_pck2<<endl;
    }

	//add sub-chunks from C onto nodes
	for(int i=0; i< nb_pck2 ; i++){
		for(int j=0; j <epsilon; j++){
            if(l<=C.size()){
			user_node[z][i].id_chunck.push_back(C[l]);
			l++;
            }
		}
	}
    int ID1 = user_node[z][0].id;
    for (int i=1; i<nb_pck2; i++){
        user_node[z][i].id= ID1+i;

    }
    /*for(int i=0;i<user_node[z].size();i++){
        for(int j=0; j<chunks_Node2[userID];j++)
        cout<<"ID= "<<user_node[z][i].id<<" chunk= "<<user_node[z][i].id_chunck[j]<<" ";
        cout<<endl;
    }
*/
    for(int i=userID+1;i<n_utenti;i++){
        if(chunks_Node[i]==chunks_Node2[i]){
            for(int j=0; j<node_list_unchanged.size();j++){
                if(node_list_unchanged[j].id_utente==i)
                    node_list_unchanged[j].id-=extra_Nodes;
            }
        }
        else{
            for(int j=0; j<user_node.size();j++){
                for(int k=0;k<user_node[j].size();k++)
                if(user_node[j][k].id_utente==i)
                    user_node[j][k].id-=extra_Nodes;
            }
        }
    }

    //cout<<endl<<"DONE ADDING SUB-CHUNK"<<endl;

}
/****************************************************************************************************************/
int *colorUpdate(){

    newColors=(int*) malloc (size_node_list * sizeof(int)); 
    cout<<"\n\nSTART COLOR UPDATE\n";

    for(int i=0; i<size_node_list;i++){
        newColors[i]=node_list_updated[i].col;
    }
    return newColors;
}



int nCol(){
    int res = 1;
 
    // Pick all elements one by one
    for (int i = 1; i < size_node_list; i++) {
        int j = 0;
        for (j = 0; j < i; j++)
            if (newColors[i] == newColors[j])
                break;
 
        // If not printed earlier, then print it
        if (i == j)
            res++;
    }

    return res;
}

/****************************************************************************************************************/
/*This is a main function*/
cf_data conflictGraphGenerator(data_matrix data, vector<int> coderate){
    m_files = data.m_files;
    b_chunks = data.b_chunks;
    n_utenti = data.n_utenti;
    Ind = data.Ind;
    Q = data.Q;
    Q_chuncks = data.Q_chuncks;
    cf_data output;

    chunks_Node = coderate;//6 because we assume the chunk size is 1/6 of the packet size
    cout << "--- Conflict Graph ---" << endl;
    if (Ind != NULL && Q != NULL && Q_chuncks != NULL){
        /*Bulding a conflict information graph*/
        computeNumberOfNodes();

        if (nodi_user[0] > 0){
            makeNodes();
            makeEdges();
        }else{
            m_archi = 0;
        }

        output.Matrix_Adj = Matrix_Adj;
        output.n_nodi = sizenodes;
        //Copy the nodes into the array node_list for compatibility with the functions in other files
        node_list = (nodo *) malloc (sizenodes * sizeof(nodo));
        if (!node_list){
            printf("\nError: Allocation Nodes.\n");
            exit(0);
        }

        cout<<"n_utenti = "<<n_utenti<<endl;
        //cout << "Nb chunks per node: \n";

        for (int i = 0; i < sizenodes; ++i){
            cout << nodes.at(i).id_chunck.size() << ",";
            node_list[i].id = nodes.at(i).id;
            node_list[i].degree = nodes.at(i).degree; //cout<<" degree= "<<nodes.at(i).degree<<endl;
            node_list[i].id_utente = nodes.at(i).id_utente;
            node_list[i].id_file = nodes.at(i).id_file;
            node_list[i].id_chunck = nodes.at(i).id_chunck;
        }
        cout << endl;

        output.nodes = node_list;
        output.Ind = Ind;

        //_dealloc();
    }else{
        output.n_nodi = 0;
    }

    return output;
}

/***************************************************************************************/
cf_data conflictGraphGeneratorUpdated(data_matrix data, vector<int> coderate, vector<int> coderate2, int *color){
    m_files = data.m_files;
    b_chunks = data.b_chunks;
    n_utenti = data.n_utenti;
    Ind = data.Ind;
    Q = data.Q;
    Q_chuncks = data.Q_chuncks;
    cf_data outputUpdated;
    int change=0;
    int z;

    chunks_Node = coderate;
    chunks_Node2 = coderate2;

    for(int i=0;i<sizenodes;i++)
        nodes[i].col=color[i];

    for(int j=0; j<n_utenti ; j++){
        //cout<<endl<<"user "<<j<<endl;
        if(coderate[j] == coderate2[j]){ // fill node_list_unchanged with nodes of users whose coderate doesnt change
            //cout<<endl<<"user "<<j<<" didnt change rate"<<endl;
            for (int i = 0; i < sizenodes; ++i){
                if(nodes.at(i).id_utente == j){
                    nodo n2;
                    n2.id=nodes.at(i).id;
                    n2.degree=0;//nodes.at(i).degree;
                    n2.id_utente=nodes.at(i).id_utente;
                    n2.id_file=nodes.at(i).id_file;
                    n2.id_chunck=nodes.at(i).id_chunck;
                    n2.col=nodes.at(i).col;
                    node_list_unchanged.push_back(n2);
                }
            }
        }else{ // fill user_node with nodes of users whose coderate changes
           // cout<<endl<<"user "<<j<<" changed rate"<<endl;
            vector<nodo> temp;
            for (int i=0; i<sizenodes ; i++){
                if(nodes.at(i).id_utente == j){
                    nodo n5;
                    n5.id=nodes.at(i).id;
                    n5.degree=0;//nodes.at(i).degree;
                    n5.id_utente=nodes.at(i).id_utente;
                    n5.id_file=nodes.at(i).id_file;
                    n5.id_chunck=nodes.at(i).id_chunck;
                    n5.col=nodes.at(i).col;
                    temp.push_back(n5);
                    /*cout<<endl<<"nodes= ";
                    for(int c=0;c<chunks_Node[j];c++)
                        cout<<nodes[i].id_chunck[c]<<" ";*/
                }
            }
            //cout<<"TEMP OK--"<<endl;
            user_node.push_back(temp);
            //cout<<endl<<"done push_back"<<endl;
            }
        //cout<<"done user "<<j<<endl; 
    }

    cout<<"\n-----------done with first step-----------"<<endl; 
            
    for(int i=0; i<n_utenti ; i++){
        if(coderate[i] > coderate2[i]){
           // cout<<endl<<"split for user "<<i<<" start"<<endl;
            for(int c=0;c<user_node.size();c++){
                if(user_node[c][0].id_utente==i){
                    z=c; break;
                }
            }
            splitNode(i, z);
            //cout<<endl<<"split of user "<<i<< " successfull"<<endl;
        }else if(coderate[i] < coderate2[i]){
                    //cout<<endl<<"merge for user "<<i<<" start"<<endl;
                    for(int c=0;c<user_node.size();c++){
                        if(user_node[c][0].id_utente==i){
                            z=c; break;
                        }
                    }
                    MergeNode(i, z);
                    //z++; 
                    //cout<<endl<<"merge of user "<<i<<" successfull"<<endl;
                }
    }

    cout<<"\nchanges on user_node done\n";
    cout<<"\ncopy all nodes onto node_list_updated\n";

    for(int u=0; u<n_utenti;u++){
        if(coderate[u]==coderate2[u]){
            for(int q=0;q<node_list_unchanged.size();q++){
                if(node_list_unchanged[q].id_utente==u) node_list_updated.push_back(node_list_unchanged[q]);
            }
        }
        else{
            for(int c=0;c<user_node.size();c++){
                if(user_node[c][1].id_utente==u){
                    z=c; break;
                }
            }
                for(int j=0;j<user_node[z].size(); j++){
                    if(user_node[z][j].id_utente==u){
                    nodo n3;
                    n3.id = user_node[z][j].id;
                    n3.degree = 0;//user_node[z][j].degree;
                    n3.id_utente = user_node[z][j].id_utente;
                    n3.id_file = user_node[z][j].id_file;
                    n3.id_chunck = user_node[z][j].id_chunck;
                    n3.col=user_node[z][j].col;
                    node_list_updated.push_back(n3);
                    }
                }
            
        }
    }

    cout<<"\n node_list_updated DONE\n";
    cout<<endl;


    size_node_list = node_list_updated.size();
    outputUpdated.n_nodi = size_node_list;

    makeEdges_ON_node_list();

    //compatability
    node_listc = (nodo *) malloc (size_node_list * sizeof(nodo));
    for (int i = 0;i< size_node_list; ++i){
        node_listc[i].id = node_list_updated.at(i).id;
        node_listc[i].degree = node_list_updated.at(i).degree; //cout<<"\n degree = "<<node_listc[i].degree;
        node_listc[i].id_utente = node_list_updated.at(i).id_utente;
        node_listc[i].id_file = node_list_updated.at(i).id_file;
        node_listc[i].id_chunck = node_list_updated.at(i).id_chunck;
    }


    outputUpdated.nodes = node_listc;
    outputUpdated.Ind = Ind;
    outputUpdated.Matrix_Adj = Matrix_Adj2;

    return outputUpdated;

}
/***************************************************************************************/
}//end namespace caching