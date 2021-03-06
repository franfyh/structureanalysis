// Read description in ShapeKnots.h

#include "ShapeKnots.h"

//Flags for output to the screen
//#undef OUTPUT_TO_SCREEN
#define OUTPUT_TO_SCREEN

// Flags for debug features
#undef DEBUG_MODE
//#define DEBUG_MODE

void addtoAggregate(structure * folded, structure * psa, int r){
	//Function that takes a folded structure (after call to dynamic) and adds it to the final list of folded structures
	//folded is the newly folded structure
	//psa is a pointer to the final structure and is equal to pseudoStructAggregate.
	//NOTE!!! only structure r is added to the list, no matter how many tracebacks there are

	psa->AddStructure();	 
	psa->SetCtLabel(folded->GetCtLabel(1),psa->GetNumberofStructures());
	psa->SetEnergy(psa->GetNumberofStructures(),folded->GetEnergy(r));
	for (int i=1;i<=folded->GetSequenceLength();i++){ 
		if (folded->GetPair(i,r)>i) psa->SetPair(i,folded->GetPair(i,r),psa->GetNumberofStructures());
	}
}

void pseudoknotFold(pkHelix &pknot, structure * st, structure * psa, int energyPrune, datatable *data, int maxtracebacks, int percent, int window, int &numstructures, double P1, double P2, double Ss, double Si, string DMSFile, string SHAPEFile, double DSs, string DSHAPEFile, string doubleOffsetFile)
//Function that removes a helix (forces all nucleotides in that helix to be single stranded) from an RNA, folds the RNA, and then 
//	adds the helix back IF the addition of the helix to the folded RNA lowers the overall energy of the RNA below the energy of the
//	pseudoknot-free minimum free energy structure. 
//pknot is a pointer to the helix to be removed from the RNA (forced to be single stranded).
//st is a pointer to the RNA structure object that will be folded.
//psa is a pointer to the RNA structure object that will hold the pseudoknot free minimum free energy structure and 
//	all folded RNA structures that contain a pseudoknot.  This is equivalent to pseudoStructAggregate.
//energyPrune is the total energy of the minimum free energy structure
//data is a pointer to a datatable object (defined by DHM, needed for the call to Dynamic)
//maxtracebacks defines number of sub-optimal structures to be included in the dynamic function
//percent defines how close in energy sub-optimal structures can be
//numstructures indicates how many RNAs are in psa

{

	//Create RNA class instance 'rna'

	//Declare character string that holds sequence from 'structure' class.
	//Set length of 'sequence' array equal to the size of the structure in 'structure' class. 
	char *sequence = new char[st->GetSequenceLength()+1];
	//Copy nucleotides from 'structure' to 'sequence'.
	for(int i=0;i<st->GetSequenceLength();++i) sequence[i]=st->nucs[i+1];
	//Insert a NUL terminator at the end of 'sequence'.
	sequence[st->GetSequenceLength()]='\0';
	//RNA class stores the structure specified by the user.	  
	//Initialize an empty RNA classes. 
	int error;//Stores error code.
	RNA *rna=new RNA(sequence);

	//If SHAPE restraints file was specified
	if(!SHAPEFile.empty()){
		//Read SHAPE data into 'rna'. The SHAPEslope and SHAPEintercept
		error=rna->ReadSHAPE(SHAPEFile.c_str(), Ss, Si);
		//If there was an error output it to the screen and exit.
		if (error!=0) {
			cerr << rna->GetErrorMessage(error);
			delete rna;
		}
	}

	//If SHAPE restraints file was specified
	if(!DSHAPEFile.empty()){
		//Read SHAPE data into 'rna'. The SHAPEslope and SHAPEintercept
		error=rna->ReadSHAPE(DSHAPEFile.c_str(), DSs, 0,"diffSHAPE");
		//If there was an error output it to the screen and exit.
		if (error!=0) {
			cerr << rna->GetErrorMessage(error);
			delete rna;
		}
	}

    //If the user has specified a DMS restraints file, read the file.
	if(!DMSFile.empty()){
		//Read in the dms data file and convert the data to a linear penalty 
        error=rna->ReadSHAPE(DMSFile.c_str(),0,0,"DMS");
        if (error!=0) {
			cerr << rna->GetErrorMessage(error);
			delete rna;
		}
	}
	
	//Read the Doublestrand offset data into 'rna'
	if(!doubleOffsetFile.empty()){
		error=rna->ReadDSO(doubleOffsetFile.c_str());//read the offset data
		if (error!=0) {
			cerr << rna->GetErrorMessage(error);
			delete rna;
		}
	}
	
	pknot.removeHelixFromStructure(st);	   //force all nts in helix to be single stranded


	
	


	while (st->GetNumberofStructures()>0) st->RemoveLastStructure();	//Remove all previous structures from st
	dynamic(st, data, maxtracebacks, percent, window);	  //fold the rest of the structure



	//determine if the energy of structure is less than that of the pseudoknot-free MFE	
	for(int r=1; r<=st->GetNumberofStructures(); r++){
		pknot.addHelixToStructure(st,r);	//add the helix back to the folded structure

		


		//Prepare RNA for the copy of new suboptimal structure by setting all pairs in rna to 0.
		//for(int i=1;i<=rna->GetSequenceLength();++i) rna->SpecifyPair(i,0);
		rna->RemovePairs();

		



		//Copy all pairs from the structure in 'structure' to RNA class 'rna'.
		for(int i=0;i<st->GetSequenceLength();++i){
			if (st->GetPair(i+1,r)>0) rna->SpecifyPair(i+1, st->GetPair(i+1,r));



		
		
		}
		



		//Fill in single or tandem mismatches in RNA structure 'rna'
		FillMismatch(rna);

		


		//Remove isolated pairs in rna structure 'rna'
		RemoveIsolatedPairs(rna);

		


		//If there is a pseudoknot:
		if(rna->ContainsPseudoknot(1)){

			

			//	if((st->energy[r]+pknot->getEnergy())<(energyPrune)){ 
			addtoAggregate(st, psa, r);			//add the entire structure to the final list of pseudoknotted structures
			//		}		
			
			
		}
	}
	delete rna;
	delete[] sequence;
}

//Print the file with helices
void printhelixListtoFile(vector<pkHelix> pkhelixList){
	//Function that prints the list of 40 helices to a text file. Useful for debugging how the list is built
	ofstream checkfile;
	checkfile.open("HelixEnergyFile.txt");
	checkfile<<"helix energy  helix_length	  helix nucleotides"<<endl;

	for(int i=0;i<pkhelixList.size();i++){
		checkfile<<setw(5)<<setiosflags(ios::right)<<pkhelixList[i].getEnergy()<<setw(15)<<pkhelixList[i].getSize()/2<<"		   ";
		for (int j=0;j<pkhelixList[i].getSize();j++){
			checkfile<<pkhelixList[i].getelement(j)<<" ";
		}
		checkfile<<endl;
	}
}


void printPseudoknotList(structure* pseudoStructAggregate){
	//Function that checks through each of the final folded structures for the presence of pseuodoknots.  Checks by iterating through
	//	all helices looking for the condition that satisfies i<i'<j<j', where i is paired to j, and i' is paired to j'.
	//This algorithm is not exhaustive in that a helix might have interleaved basepairs with more than one helix.  Therefore,
	//	manual inspection of any structure which contains interleaved basepairs will have to be examined.
	//pseudoStructAggregate is the final list of structures, each structure is individually checked.
	ofstream cck;
	cck.open("PseudoknotOutput.txt");
	bool pseudoknot = false;
	for(int l=1; l<=pseudoStructAggregate->GetNumberofStructures(); l++){
		pseudoknot = false;
		for( int r=1;r<=pseudoStructAggregate->GetSequenceLength();r++){
			if(pseudoStructAggregate->GetPair(r,l)!=0 && pseudoknot == false){
				for(int x=1;x<pseudoStructAggregate->GetSequenceLength();x++){
					if(pseudoStructAggregate->GetPair(x,l)!=0 && pseudoknot == false){
						if( ( (r< x) &&(x < pseudoStructAggregate->GetPair(r,l)) &&( pseudoStructAggregate->GetPair(r,l) < pseudoStructAggregate->GetPair(x,l)) )||
						    ( (x< r) &&(r < pseudoStructAggregate->GetPair(x,l))&& (pseudoStructAggregate->GetPair(x,l)<pseudoStructAggregate->GetPair(r,l)) )	){
							pseudoknot = true;
							//cout<<"In structure "<<l<<", pseudoknot between helix starting with basepair "<<r<<"-"<<pseudoStructAggregate->basepr[l][r]<<endl<<
							//	" and helix starting with basepair "<<x<<"-"<<pseudoStructAggregate->basepr[l][x]<<endl;
							//cck<<"In structure "<<l<<", pseudoknot between helix starting with basepair "<<r<<"-"<<pseudoStructAggregate->basepr[l][r]<<endl<<
							//	" and helix starting with basepair "<<x<<"-"<<pseudoStructAggregate->basepr[l][x]<<endl;
						}			
					}
				}
			}
		}
	}
}


void convertToHundredHelices(vector<pkHelix> &pkHelixList, int finallistSize){
	//Function that "trims" the list of possible pseudoknots to the 100 (or 'finallistSize') most energetically stable (lowest energy).
	// Initial list can be a few to several thousand possibilities depending on the size of the RNA.
	//pkHelixList is a pointer to the list of helices, a list of pkHelix objects.
	//pkHelisListSize is the current size of the list of helices
		
	//if number of possible pseudoknots is less than this, return
	//QUESTION: Should I changed the '<' to '<='
	if(pkHelixList.size()<finallistSize){
#ifdef DEBUG_MODE
		printhelixListtoFile(pkHelixList);
#endif
		return;
	}

	//start at position finallistSize (default = 100)
	for (int i=finallistSize; i<pkHelixList.size(); i++){
		//examine the energy of the first finallistSize (defalut = 100) helices compared to each helix after finallistSize
		for (int j=0;j<finallistSize; j++){			
			//if the energy of the ith helix is less than the energy of the jth helix, swap 
			if ((pkHelixList[i].getEnergy())<(pkHelixList[j].getEnergy())){
				pkHelix temppkhelix (pkHelixList[i]);
				pkHelixList[i].setEqual(pkHelixList[j]);
				pkHelixList[j].setEqual(temppkhelix);
			}
		}
	}
	pkHelixList.erase (pkHelixList.begin()+finallistSize,pkHelixList.end());
	//pkHelixList.size()=finallistSize;
#ifdef DEBUG_MODE
	printhelixListtoFile(pkHelixList);
#endif
}

void findhelix(short **arrayPointer, vector<pkHelix>  &pkHelixList, int x, int y, datatable *data, structure *ct){
	//Function to find helices in the 2-d array generated fill.	 
	//arrayPointer is the pointer to the 2-d array.
	//pkHelixList is a pointer to the list of helices, a list of pkHelix objects.
	//pkHelisListSize is the current size of the list of helices
	//x and y are coordinates in the 2-d array
	//data is a pointer to a data object defined by DHM, passed to the pkHelix constructor and ultimately used to calculate the energy of the new helix
	//ct is a pointer to a structure object defined by DHM, passed to the pkHelix constructor and ultimately used to calculate the energy of the new helix
	//This function is a little awkward.  Takes a cell (x,y) identified to hold a base pair in the defineHelixList function and looks to see if there are more
	//	basepairs on the diagonal with the same energy, indicating a helix.	 Only helices greater than 2nts and less than 10 nts are considered to be possible pseudoknots.
	short helixLength = 1;
	
	//make sure that the next cell on the diagonal exists first
	//by subtracting the Y value from the X value...if at the end, the result will be less than 2
	//return without changing anything.
	//check to see if the next element on the diagonal is equal to calling cell
	while(	(((x-y)>2)||((x-y)<-2)) && (arrayPointer[x][y] == arrayPointer[x-1][y+1] ) ){
		arrayPointer[x][y] = 28001;	  //reset the current cell so it won't be found again 
		x--;						  //decrement x to be tested next while loop	
		y++;						  //increment y to be tested next while loop	
		helixLength++;
	}
	//make sure that the calling helix is  greater than two nts and less than 10 nts
	if (helixLength>2){
		arrayPointer[x][y] = 28001; //set the last element of the helix to 28001
		//create an array of shorts to hold numbers involved in helix
		//use standard array notation 0 to n-1
		short *temphelix;
		//temphelix = new short [2*helixLength-1];//DELETED
		temphelix = new short [2*helixLength];//ADDED
		//fill the temporary array of shorts backward since variables hold last elements of helix
		for (int i =2*helixLength-1; i>0;i--){//ADDED
			temphelix[i]=x;
			temphelix[i-1]=y;
			x++;
			y--;
			i--; //filling two elements at a time, so decrement i again
		}

		//create a temporary pkHelix object to hold new helix
		//pkHelix temppkhelix(data, ct, (2*helixLength), temphelix);
		//assign temporary pkHelix object to next empty position in list
		//QUESTION: Memory Leak Here:		
		//pkHelixList.push_back(temppkhelix);
		pkHelixList.push_back(pkHelix(data, ct, (2*helixLength), temphelix));
		delete[] temphelix;	
	}
}

void defineHelixList(short **arrayPointer, vector<pkHelix> &pkHelixList, int & maxsize, int lowvalue, datatable *data, structure *ct){
	//Function that iterates through the 2-d array generated by the fill function, and looks for structures with total energy within 25% of the pseudoknot-free minimum free energy structure.
	//arrayPointer is the pointer to the 2-d array.
	//pkHelixList is a pointer to the list of helices, a list of pkHelix objects.
	//pkHelisListSize is the current size of the list of helices
	//data is a pointer to a data object defined by DHM, passed to findhelix function and then the pkHelix constructor and ultimately used to calculate the energy of the new helix
	//ct is a pointer to a structure object defined by DHM, passed to findhelix function and then the pkHelix constructor and ultimately used to calculate the energy of the new helix
	//max size is the maximum size of the list of helices -	 resized as needed
	//lowvalue is the energy of the pseudoknot-free minimum free energy structure.	This was calculated in fill and stored when the 2-d array was built
//<<<<<<< ShapeKnots.cpp
//	int upperEnergyValue;
//	if(ct->GetSequenceLength()>100)
//=======
	double upperEnergyValue;
	if(ct->GetSequenceLength()>100)
//>>>>>>> 1.11
		upperEnergyValue =( lowvalue-(lowvalue*0.25));		//defines upper end of energy range when looking through helices in other structures
	else
		upperEnergyValue =( lowvalue-(lowvalue*0.999));
	//set upper energy range to be withing 25% of pseudoknot-free MFE structure
	//now iterate through the array looking for matrix positions that might be good starting points for the pseudoknot
	for (int i=1;i<=ct->GetSequenceLength(); i++){
		for(int j=i;j<=ct->GetSequenceLength(); j++){
			//checks the following in order:
			//are the bases less than 600 nt apart?
			//is the energy greater than pseudoknot-free MFE, this helps prevent false positives, 
			//position greate and make sure the position is not right at the diagonal 
			if((j-i<=600)&&(arrayPointer[j][i]<=upperEnergyValue)&& (arrayPointer[j][i]>lowvalue) && ( ((i-j)>2) ||( (i-j)<-2) ) ){
				//once a position is found, pass the findhelix function the 2-d array, the 3-d array to be populated and the positions of the cell
				//REMEMBER using j to iterate along the X-axis and i to iterate along Y-axis
				
				findhelix(arrayPointer, pkHelixList, j, i, data, ct);
			}
		}
	}		
}

void convertStructureToHelixList(structure *st, datatable *data, vector<pkHelix> &structureHelices){
	//Function that converts the pseudoknot-free MFE (OR ANY STRUCTURE) to a list of helices.  This function uses a pointer to a list of pkHelix objects 
	//	to hold the list.  This list will use this to compare MFE helices to proposed pseudoknotted structure to look for false positives
	//	Only looks at lowest energy structure of each traceback, will only report first structure anyway
	//st is a pointer to a structure object, holds all data associated with an RNA structure 
	//data is a pointer to datatable object 
	//structureHelices is a pointer to the list of helices in the pk-free MFE
	//structureHelicesSize is the number of helices in the list
   
	//temporary object to hold new list
	int thlength = 0;
	short *temphelix;

	temphelix = new short [st->GetSequenceLength()];
	//look through the array that holds the list of basepairs in a structure, look for helices.
	for (int i=1; i<=st->GetSequenceLength(); i++){
		//look for a base pair and only pick it up once by only taking it if is greater than position
		if( st->GetPair(i)>i ){
			(temphelix[thlength++]=i); 
			(temphelix[thlength++]=st->GetPair(i));
						
			//statement that checks that helix is closed properly
			if(	 st->GetPair(i+1) != st->GetPair(i)-1 ){
				structureHelices.push_back(pkHelix(data,st,thlength,temphelix));
				thlength=0;
			}//end of end of helix - inside loop finding basepair >i
		}//end of find basepair >i
	}//end of for loop
	delete[] temphelix;
}//end of function

void comparepkHelixListToMFE(vector<pkHelix> &pkHelixList, structure *ct, vector<pkHelix> &MFEhelixList){
	//Function that removes any helices from the list of possible pseudoknots that will significantly repair the pseudknot-free MFE.
	//this is simply an empirically derived method to reduce the number of false positives in the prediction
	//pkHelixList is a pointer to the list of helices, a list of pkHelix objects. tHIS IS THE LIST OF POSSIBLE PSEUDOKNOTS
	//pkHelisListSize is the current size of the list of possible pseudoknots
	//ct is a pointer to a structure object, holds all data associated with an RNA structure (programmed by DHM)
	//data is a pointer to datatable object (programmed by DHM)
	//MFEhelixlist is a pointer to a list of helices, a list of pkHelix objects.  THIS IS THE LIST OF HELICES IN THE PK-FREE MFE.
	//MFEhelixListSize is the number of helices in the pk-free MFE
	bool same =false;
	//first iterate through the list of helices in the pk-free MFE and remove all helices that are less than 3 basepairs.
	//But keep all helices that have 1nt bulges
	//Again, just an empirical rule that allows repairing of short helices
	for(int y=0; y<MFEhelixList.size(); y++){
		if((MFEhelixList[y].getSize()<=6)&&((ct->GetPair((MFEhelixList[y].get5primeLast())+2))+1!=(ct->GetPair((MFEhelixList[y].get5primeLast()))))
		   &&((ct->GetPair((MFEhelixList[y].get3primeLast())+2))+1!=(ct->GetPair((MFEhelixList[y].get3primeLast()))))  
		   &&((ct->GetPair((MFEhelixList[y].get5primeFirst())-2))-1!=(ct->GetPair((MFEhelixList[y].get5primeFirst()))))
		   &&((ct->GetPair((MFEhelixList[y].get3primeFirst())-2))-1!=(ct->GetPair((MFEhelixList[y].get5primeFirst())))) ){
			for(int z=0; z<MFEhelixList[y].getSize();z+=2){
				ct->RemovePair(MFEhelixList[y].getelement(z));
				//ct->RemovePair(MFEhelixList[y].getelement(z+1));//RemovePair function takes care of this automatically.
			}
		}
	}
	int removefirst= 0;
	int removesecond=0;//indicates the number of times nucleotides in each half of possible pseudoknotted helix overlaps MFE helices
	//If more than half of nucleotides in both halves of possible pseudoknotted helix are involved in helices in pk-free MFE, 
	//the helix significantly repairs MFE, thereforeremove helix from list
	
	//iterate through the list of possible pseudoknots
	for (int i=0; i<pkHelixList.size(); i++){
		//compare against all helices in pseudoknot free MFE structure
		for(int j=0; j<pkHelixList[i].getSize(); j+=2){
			if(ct->GetPair((pkHelixList[i].getelement(j)))!=0){
				removefirst++;
				
			}
			if(ct->GetPair(pkHelixList[i].getelement(j+1))!=0){
				removesecond++;
				
			}
		}//end of j
		//check if more than half of nucleotides in both 5' and 3' halves of possible pseudoknot are basepaired in pk-free MFE
		//note that checking the possible pseudoknotted helix, not the helix in the pk-free MFE
		if((removefirst>pkHelixList[i].getSize()/4)&&(removesecond>pkHelixList[i].getSize()/4)){
			same=false;
			
			for(int f=0; f<MFEhelixList.size();f++){
				if(pkHelixList[i].isEqual(MFEhelixList[f]))
					same=true;
			}
			if(same==false){
				pkHelixList.erase(pkHelixList.begin()+i);
				i--;
			}
		}
		removefirst=removesecond=0;
		
	}//end of i

}


//Check for duplicate structures in a structure class and remove them.
void checkForDuplicates(structure* &pseudoStructAggregate){
	//function that checks the final folded list of structures to ensure that no structure is duplicated
	//pseudostructAggregate is a pointer to the final list of folded structures.
	bool same;
	for(int l=1; l<pseudoStructAggregate->GetNumberofStructures(); l++){

		same=true;
		for(int r=l+1;r<pseudoStructAggregate->GetNumberofStructures();r++){
			//only check structures which have the same energy
			//if (pseudoStructAggregate->energy[l]==pseudoStructAggregate->energy[r]){
			same=true;
			for (int h=1; h<=pseudoStructAggregate->GetSequenceLength();h++){		
				//if the basepair is not equal at any point, then structures are not equivalent
				//can stop comparing these two structures and break back to r
				if(pseudoStructAggregate->GetPair(h,l)!=pseudoStructAggregate->GetPair(h,r)){
					same = false;						
					break;
				}//end of if
			
			}//end of j
			//will either come out of j with same equal to true or false
			//if same equals true want to exit r and go back to i same is equal to false if the break occurs
			//if same is still true after going all the way through any of them, want to skip this i position and go to the next
			if(same==true){
			
				//Use INIFINTE_ENERGY as a flag for duplicates
				pseudoStructAggregate->SetEnergy(l,INFINITE_ENERGY);
			
				break;	//break out of r
			}
			//	}//end of if energy is the same
			
		}	
		
	}//end of R	


	//Use the flag of high folding free energy to put all duplicates at the end
	//of the list of structures by sorting and then remove them.
	pseudoStructAggregate->sort();
	for (int i=pseudoStructAggregate->GetNumberofStructures();i>0;--i) {
		if (pseudoStructAggregate->GetEnergy(i)==INFINITE_ENERGY) {
			pseudoStructAggregate->RemoveLastStructure();
		}
		else {
			break;
		}
	}
}		
void copyStructure(structure * ct, structure* &pseudoStructAggregate, string DMSFile, string SHAPEFile){
	//Function that initializes pseudoStructAggregate to have some of the same attributes as the original structure ct
	//ct is allocated in main, and reads in the initial data
	//pseudoStructAggregate is a pointer to a structure that holds the final list of folded structures

	if (ct->DistanceLimited()) pseudoStructAggregate->SetPairingDistance(ct->GetPairingDistanceLimit());		//only allow base pairs closer than 600
	
	pseudoStructAggregate->allocate(ct->GetSequenceLength()); //indicates the size of the new structure
	if(!SHAPEFile.empty()||!DMSFile.empty()){
		pseudoStructAggregate->SHAPE = new double [2*ct->GetSequenceLength()+1];
		pseudoStructAggregate->SHAPEss = new double [2*ct->GetSequenceLength()+1];
		
		pseudoStructAggregate->SHAPEss_region = new short int *[pseudoStructAggregate->GetSequenceLength() + 1];
		for (int i = 1; i <= pseudoStructAggregate->GetSequenceLength(); i++) {
			pseudoStructAggregate->SHAPEss_region[i] = new short int [i];
			for (int j=0;j<i;++j) pseudoStructAggregate->SHAPEss_region[i][j]=0;
		}

	}
	
	if(!SHAPEFile.empty()||!DMSFile.empty()) pseudoStructAggregate->shaped=true;
	else pseudoStructAggregate->shaped=false;
	
	//set the identifier for the structure 
	for (int q=1;q<=ct->GetSequenceLength();q++){	
		pseudoStructAggregate->numseq[q] = ct->numseq[q];	//set to equivalent position in original structure
		pseudoStructAggregate->hnumber[q]= ct->hnumber[q];
		pseudoStructAggregate->nucs[q] = ct->nucs[q];
		if(!SHAPEFile.empty()||!DMSFile.empty()){
			pseudoStructAggregate->SHAPE[q]=ct->SHAPE[q];		
			pseudoStructAggregate->SHAPEss[q]=ct->SHAPEss[q];
		}
	}
	if(!SHAPEFile.empty()||!DMSFile.empty()){
		pseudoStructAggregate->SHAPEslope=ct->SHAPEslope;
		pseudoStructAggregate->SHAPEintercept=ct->SHAPEintercept;
	}
}


//	Function gets the names of data files to open
void getdat(char *loop, char *stackf, char *tstackh, char *tstacki,
            char *tloop, char *miscloop, char *danglef, char *int22,
            char *int21,char *coax, char *tstackcoax,
            char *coaxstack, char *tstack, char *tstackm, char *triloop,
            char *int11, char *hexaloop, char *tstacki23, char *tstacki1n,
            char *datapath, bool isRNA)

{

	if( !isRNA) strcat( datapath,"dna");
	strcpy (loop,datapath);
	strcpy (stackf,datapath);
	strcpy (tstackh,datapath);
	strcpy (tstacki,datapath);
	strcpy (tloop,datapath);
	strcpy (miscloop,datapath);
	strcpy (danglef,datapath);
	strcpy (int22,datapath);
	strcpy (int21,datapath);
	strcpy (triloop,datapath);
	strcpy (coax,datapath);
	strcpy (tstackcoax,datapath);
	strcpy (coaxstack,datapath);
	strcpy (tstack,datapath);
	strcpy (tstackm,datapath);
	strcpy (int11,datapath);
	strcpy (hexaloop,datapath);
	strcpy (tstacki23,datapath);
	strcpy (tstacki1n,datapath);
 
	strcat (loop,"loop.dat");
	strcat (stackf,"stack.dat");
	strcat (tstackh,"tstackh.dat");
	strcat (tstacki,"tstacki.dat");
	strcat (tloop,"tloop.dat");
	strcat (miscloop,"miscloop.dat");
	strcat (danglef,"dangle.dat");
	strcat (int22,"int22.dat");
	strcat (int21,"int21.dat");
	strcat (triloop,"triloop.dat");
	strcat (coax,"coaxial.dat");
	strcat (tstackcoax,"tstackcoax.dat");
	strcat (coaxstack,"coaxstack.dat");
	strcat (tstack,"tstack.dat");
	strcat (tstackm,"tstackm.dat");
	strcat (int11,"int11.dat");
	strcat (hexaloop,"hexaloop.dat");
	strcat (tstacki23,"tstacki23.dat");
	strcat (tstacki1n,"tstacki1n.dat");
}

void pseudoknot(structure* ct, datatable *data, int maxStructures, int percent, int windowSize, string  ctFile, double P1, double P2, double Ss, double Si, string DMSFile, string SHAPEFile, double DSs, string DSHAPEFile, string doubleOffsetFile, int OutPercent, int OutWindowSize, int OutMaxStructures, bool ifWindowOptions, int finallistSize)
//Function that drives the rest of the code.  This function is called in main after the sequence file, shape file and shape parameters are
//	are read.
//ct is a pointer to a structure object, holds all data associated with an RNA structure (programmed by DHM)
//data is a pointer to datatable object (programmed by DHM)
//maxtracebacks defines number of sub-optimal structures to be included in the dynamic function
//percent defines how close in energy sub-optimal structures can be
//ctFile is the name of the file where results are printed
//SHAPEslope and SHAPEintercept are the SHAPE parameters
{
	integersize *w5,*w3;
	bool *lfce,*mod;
	//int crit = 0;//DELETED because unised
	int i = 0;
	int j = 0;
	int vmin;
	arrayclass *w2,*wmb2;
	arrayclass *w, *v, *wmb;
	forceclass *fce;
	w = new arrayclass(ct->GetSequenceLength());
	v = new arrayclass(ct->GetSequenceLength());
	wmb = new arrayclass(ct->GetSequenceLength());
	fce = new forceclass(ct->GetSequenceLength());

	lfce = new bool [2*ct->GetSequenceLength()+1];
	mod = new bool [2*ct->GetSequenceLength()+1];

	for (i=0;i<=2*ct->GetSequenceLength();i++) {
		lfce[i] = false;
		mod[i] = false;
	}

	w5 = new integersize [ct->GetSequenceLength()+1];
	w3 = new integersize [ct->GetSequenceLength()+2];

	for (i=0;i<=ct->GetSequenceLength();i++) {
		w5[i] = 0;
		w3[i] = 0;

	}
	w3[ct->GetSequenceLength()+1] = 0;

	if (ct->intermolecular) {
		w2 = new arrayclass(ct->GetSequenceLength());
		wmb2 = new arrayclass(ct->GetSequenceLength());
	}
	else {
		w2 = NULL;
		wmb2 = NULL;
	}
	
	force(ct,fce,lfce);
	vmin=DYNALIGN_INFINITY;
#ifdef OUTPUT_TO_SCREEN
	cout << "\t\t\t\tDONE\nGenerating energy dotplot..." << flush;
#endif
	//perform the fill steps:(i.e. fill arrays v and w.)
	fill(ct, *v, *w, *wmb, *fce, vmin,lfce, mod,w5, w3, false, data, w2, wmb2);
#ifdef OUTPUT_TO_SCREEN
	cout << "\t\t\tDONE\n" << flush;
#endif
	//create the 2d array using the first dimension as an array of pointers to short
	//the second dimension is then an array of shorts which grows with each allocation +1 to create an unequal or triangular shape
	
	short **arrayPointer;	 //allocate the pointer to the 2d array for the dot plot
	arrayPointer = new short *[ct->GetSequenceLength()+1];
	for (int t =1; t<=ct->GetSequenceLength();t++)
		arrayPointer[t]=new short [t+1];

	/*This algorithm calculates the lowest dG structure containing the i-j pair such that i is less than j
	  is based on calculating the energy of the helix internal to the pair (i,j) (v.f(i,j)
	  and the energy for the part of the helix external to the pair (i,j) as v.f(j,i+ct->GetSequenceLength())
	  this function indexes the first argument as the lower
	  therefore iterate through the 2d array and fill it with this calculation:
	*/
	int lowvalue = 0;	//holds energy of the pseudoknot free MFE
	//i MUST START AT ONE 
	for(i=1; i<=(ct->GetSequenceLength()); i++){
		
		for(j=i;j<=ct->GetSequenceLength();j++){
			arrayPointer[j][i] = (v->f(i,j)+ (v->f(j,i+ct->GetSequenceLength())) );
			if (arrayPointer[j][i]<lowvalue)
				lowvalue=arrayPointer[j][i];
		}
			
	}

	//set up the list that will hold all helices that may be involved in a pseudoknot

	int maxsize=10;
	vector<pkHelix> pkhelixList;

	//this function creates a 2-d array that is a mirror of that created in fill function, and iterates through it
	//looking for starting points for helices
	defineHelixList(arrayPointer, pkhelixList, maxsize, lowvalue, data, ct);
  
	for(j=1;j<=ct->GetSequenceLength();j++) delete[] arrayPointer[j];
	delete []arrayPointer;
	//pkhelisList is now full WITH ALL POSSIBLE HELICES GREATER THAN 2NTS AND LESS THAN 10 NTS 
	//this list will be trimmed down to a manageble size once the pseudoknot-free MFE is folded

	//structure that will hold the final list of pseudoknotted helices
	//all pseudoknotted structures will be added to this structure
	structure* pseudoStructAggregate= new structure;
	copyStructure(ct, pseudoStructAggregate, DMSFile, SHAPEFile);

#ifdef OUTPUT_TO_SCREEN
	cout << "Folding the MFE structure..." << flush;
#endif
	//fold the pseudoknot free MFE structure
	dynamic(ct,data,maxStructures,percent,windowSize);
#ifdef OUTPUT_TO_SCREEN
	cout << "\t\t\tDONE\n" << flush;
#endif

	//will add all structures indicated by the user with maxStructures

	for (int h=1; h<=ct->GetNumberofStructures();h++)
		addtoAggregate(ct, pseudoStructAggregate,h);

	//Now want to "trim" the list of all possible helices to a manageble size
	//First will remove any helices that significantly repair the pseudoknot free MFE
	//only compare list of possible pseudoknots to pseudoknot-free MFE if RNA is long enough to generate more than 10 possibilities 
	if(pkhelixList.size()>10){
		//2-d list to store the pseudoknot free structure as a list of helices -  will be used later to test for psuedoknot false positives
		//pkHelix *MFEhelixList= new pkHelix[ct->GetSequenceLength()];
		vector<pkHelix> MFEhelixList(0);
		//		int MFEhelixListSize = 0;
		convertStructureToHelixList(ct, data, MFEhelixList);
		comparepkHelixListToMFE(pkhelixList, ct, MFEhelixList);
	}

	//Convert the list of helices to 100, this is final list of helices to be tested 
	//These helices will be tested for the ability to form pseudoknots by forcing all nucleotides in each 
	//to be single stranded.

	convertToHundredHelices(pkhelixList, finallistSize);

	//now test each helix for its ability to form a pseudoknot
	int numstructures=0;

	



	//iterate through the list of possible pseudoknots
	for (i=0;i<pkhelixList.size();i++){
#ifdef OUTPUT_TO_SCREEN
		cout << '\r' << "Folding modified structure "<<(i+1)<<" of "<<pkhelixList.size()<< flush;
#endif
		//fold the structure while forcing each nt in the current helix to be single stranded, and then add the helix back 
		pseudoknotFold(pkhelixList[i], ct, pseudoStructAggregate, lowvalue, data, maxStructures, percent, windowSize, numstructures, P1, P2, Ss, Si, DMSFile, SHAPEFile, DSs, DSHAPEFile, doubleOffsetFile);

		

	}
#ifdef OUTPUT_TO_SCREEN
	cout << "\t\tDONE\nChecking for duplicate structures..." << flush;
#endif
	



	//check list of folded structures for duplicates
	checkForDuplicates(pseudoStructAggregate);




#ifdef OUTPUT_TO_SCREEN
	cout << "\t\tDONE\n" << flush;
#endif



    double pseud_param[16];//Array that holds pseudoknot penalty calculation constants.
    //Read pseudoknot parameters from the datapath
    ReadPseudoParam(pseud_param);

 	int pseudnum=0;
	//Declare character string that holds sequence from 'structure' class.
	//Set length of 'sequence' array equal to the size of the structure in 'structure' class. 
	char *sequence=new char[pseudoStructAggregate->GetSequenceLength()+1];
	//Copy nucleotides from 'structure' to 'sequence'.
	for(int i=0;i<pseudoStructAggregate->GetSequenceLength();++i) sequence[i]=pseudoStructAggregate->nucs[i+1];
	//Insert a NUL terminator at the end of 'sequence'.
	sequence[pseudoStructAggregate->GetSequenceLength()]='\0';
	//RNA class stores the structure specified by the user.	  
	//Initialize an empty RNA classes. 
	int error;//Stores error code.
	RNA *rna=new RNA(sequence);

	if(!SHAPEFile.empty()){
		//Read SHAPE data into 'rna'. 
		error=rna->ReadSHAPE(SHAPEFile.c_str(), Ss, Si);
		//If there was an error output it to the screen and exit.
		if (error!=0) {
			cerr << rna->GetErrorMessage(error);
			delete rna;
		}
    }
	if(!DSHAPEFile.empty()){
		//Read SHAPE data into 'rna'. 
		error=rna->ReadSHAPE(DSHAPEFile.c_str(), DSs, 0,"diffSHAPE");
		//If there was an error output it to the screen and exit.
		if (error!=0) {
			cerr << rna->GetErrorMessage(error);
			delete rna;
		}
	}

    //If the user has specified a DMS restraints file, read the file.
	if(!DMSFile.empty()){
		//Read in the shape data file and convert the data to a linear penalty 
        error=rna->ReadSHAPE(DMSFile.c_str(), 0, 0, "DMS");
        //If there was an error output it to the screen and exit.
		if (error!=0) {
			cerr << rna->GetErrorMessage(error);
			delete rna;
		}
	}


	//Read the Doublestrand offset data into 'rna'
	if(!doubleOffsetFile.empty()){
		error=rna->ReadDSO(doubleOffsetFile.c_str());//read the offset data and store the error code in 'error'
		if (error!=0) {
			cerr << rna->GetErrorMessage(error);
			delete rna;
		}
	}

//<<<<<<< ShapeKnots.cpp
//	if(!doubleOffsetFile.empty()){
//		error=rna1->ReadDSO(doubleOffsetFile.c_str());//read the offset data and store the error code in 'error'
//		if (error!=0) {
//			cerr << rna->GetErrorMessage(error);
//			delete rna;
//		}
//	}
	
	//Initialize the structure class 'SuboptimaLpseudoStructAggregate" that will hold one suboptimal structure
	//from 'pseudoStructAggregate'
//	structure* SuboptimaLpseudoStructAggregate= new structure;
	//Copy all the parameters from 'ct' to 'SuboptimaLpseudoStructAggregate'
//	copyStructure(ct, SuboptimaLpseudoStructAggregate, SHAPEFile);

	//SuboptimaLpseudoStructAggregate->ReadOffset(NULL,DSO);//read the offset data

//	int numofstr = pseudoStructAggregate->GetNumberofStructures();
//=======
	int numofstr = pseudoStructAggregate->GetNumberofStructures();
//>>>>>>> 1.11
	//For every structure in pseudoStructAggregate execute the following:
	for(int strnum=1;strnum<=pseudoStructAggregate->GetNumberofStructures();++strnum){
#ifdef OUTPUT_TO_SCREEN
		if(strnum%100==0||strnum==numofstr) cout << "\rCalculating energy of structure " << strnum << " of " << numofstr << flush;
#endif
		//Prepare RNA for the copy of new suboptimal structure by setting all pairs in rna to 0.
		//for(int i=1;i<=rna->GetSequenceLength();++i) rna->SpecifyPair(i,0);
		if (strnum>1) rna->GetStructure()->CleanStructure(1);

//<<<<<<< ShapeKnots.cpp
		//Copy all pairs from the structure in 'structure' to RNA class 'rna' and 'rna1'.
//		for(int i=0;i<pseudoStructAggregate->GetSequenceLength();++i){
//			rna->SpecifyPair(i+1, pseudoStructAggregate->GetPair(i+1,strnum));
//			rna1->SpecifyPair(i+1, pseudoStructAggregate->GetPair(i+1,strnum));
//=======
		//Copy all pairs from the structure in 'structure' to RNA class 'rna'.
		for(int i=0;i<pseudoStructAggregate->GetSequenceLength();++i){
			if (pseudoStructAggregate->GetPair(i+1,strnum)>0) rna->SpecifyPair(i+1, pseudoStructAggregate->GetPair(i+1,strnum));
//>>>>>>> 1.11
		}

        //Do the preprocessing of rna by filling single and tandem mismatches and removing
        //isolated pairs

        //Fill in single or tandem mismatches in rna structure 'rna'
        FillMismatch(rna);
        
        //Remove isolated pairs in rna structure 'rna'
        RemoveIsolatedPairs(rna);
        
		//If there is a pseudoknot in rna:
		if(rna->ContainsPseudoknot(1)){ 
//<<<<<<< ShapeKnots.cpp
			//Copy the suboptimal structure number 'strnum' from 'pseudoStructAggregate' to SuboptimaLpseudoStructAggregate'
//			for(int i=0;i<pseudoStructAggregate->GetSequenceLength();++i){
//				SuboptimaLpseudoStructAggregate->SetPair(i,pseudoStructAggregate->GetPair(i,strnum)); 
//			}
//=======
//>>>>>>> 1.11
			++pseudnum; 
			//Calculate pseudoknot energy penalty
			double energyPenalty = pkEnergyCalc(rna, pseud_param, P1, P2);
			//Since energies stored in 'structure' class are in tenth precision, energyPenalty needs to be converted to tenth precision.
			pseudoStructAggregate->SetEnergy(strnum,pseudoStructAggregate->GetEnergy(strnum)+(energyPenalty*conversionfactor));
		}
	}
	
	//Deleting instances of RNA.
	delete rna;

#ifdef DEBUG_MODE
	cerr << '\n'<< pseudnum << " structures contain pseudoknot \n";
	cerr << pseudoStructAggregate->numofstructures-pseudnum << " structures don't contain pseudoknot \n" ;
#endif

	



	//Sort the generated structures based on their energy
	pseudoStructAggregate->sort();
#ifdef OUTPUT_TO_SCREEN
	cout << "\tDONE\nSorting structures by their free energy..." << flush;
#endif

	//If the OutWindowSize option was not specified by the user on the command line
	if(!ifWindowOptions){
		//Scale OutWindowSize based on the length of the sequence
		if(pseudoStructAggregate->GetSequenceLength()>1200) OutWindowSize=20;
		else if(pseudoStructAggregate->GetSequenceLength()>800) OutWindowSize=15;
		else if(pseudoStructAggregate->GetSequenceLength()>500) OutWindowSize=11;
		else if(pseudoStructAggregate->GetSequenceLength()>300) OutWindowSize=7;
		else if(pseudoStructAggregate->GetSequenceLength()>120) OutWindowSize=5;
		else if(pseudoStructAggregate->GetSequenceLength()>50) OutWindowSize=3;
		else OutWindowSize=2;
	}

	//Filter the output to a more reasonable number of structures
	filter(pseudoStructAggregate, OutPercent, OutMaxStructures, OutWindowSize);

	//Store the path to output CT file
	pseudoStructAggregate->ctout(ctFile.c_str());

#ifdef OUTPUT_TO_SCREEN
	cout << "\tDONE\n" << flush;
	cout << "############################################\n";
	cout << "                    DONE\n";
	cout << "############################################\n" << flush;
#endif

#ifdef DEBUG_MODE
	//Output the pseudoknot list
	printPseudoknotList(pseudoStructAggregate);
#endif

	delete[] lfce;
	delete[] mod;
 
	delete w;
	delete v;
	delete wmb;
	delete fce;
 

	delete[] w5;
	delete[] w3;

	if (ct->intermolecular) {
		delete w2;
		delete wmb2;
	}
	delete pseudoStructAggregate;
	delete[] sequence;
}
