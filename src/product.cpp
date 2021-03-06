#include "product.h"
#include "token.h"
#include "common.h"
#include "combination.h"
#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <sstream>
#include <functional>
#include <unordered_map>
#include <chrono>
#include <math.h>

Product::Product(string vendor, string title) {
    // Set the attributes of of the product
    this->vendor = vendor;
    this->title = title;
    this->preparation();
}

void Product::preparation() {
    // Runs the necessary functions to generate tokens and correctly format them according to UPM 
    Combinations.reserve(61000);
    generate_tokens();
    token_concatenator();
    generate_token_objects();
    Common::lt += Tokens.size();
    Common::lprod += 1;
    Common::lt_avg = (double) Common::lt / Common::lprod;
    Common::K = Common::lt_avg / 2;
    Common::semantic2 = 0;
}

void Product::execute() {
    // Does the main algorithmic processes
    generate_token_map();
    generate_combinations(Common::K);
}

void Product::generate_tokens() {
    // Appends the tokens tokenized_title vector

    // Check1 is used to parse the string by space 
    stringstream check1(this->title);
    string temp;
    int added = 0;
    // Goes through each token and gets rid of the punctuation from the token
    while (getline(check1, temp, ' ') && added < 100) {
        added++;
        remove_if(temp.begin(), temp.end(), ::isspace);
        for(char character : Common::punctuations) {
            temp.erase(remove(temp.begin(), temp.end(), character), temp.end());
        }

        // Converts the token to lower case letters
        transform(temp.begin(), temp.end(), temp.begin(), ::tolower);

        // Makes sure no duplicates are added to the title
        if (tokenized_title.size() != 0) {
            
            // If add is true, add the token to the tokenized_title
            bool add = true;
            for (string x : tokenized_title) {
                
                // If the same token is found, set add to false and don't add it to the tokenized_title
                if(temp == x) {
                    add = false;
                    break;
                }
            }

            // Add to the tokenized_title if add is true
            if (add)
                tokenized_title.push_back(temp);
        }

        else {
            // For the first word in title add it to the tokenized_title
            tokenized_title.push_back(temp);
        }

    }
}

void Product::token_concatenator() {
    // Makes sure that attributes that are "alone" are appended to the previous value
    for(int i = 0; i < tokenized_title.size(); i++) {
        for(string attribute : Common::attributes) {
            if(tokenized_title[i] == attribute) {
                tokenized_title[i - 1].append(attribute);
                tokenized_title.erase(tokenized_title.begin() + i);
            }
        }
    }
}

void Product::generate_token_objects() {
    // For every string token in the tokenized_title, send it to add and put the return value in Tokens
    for (string token_value : tokenized_title) {
        bool add = true;

        for (string useless : Common::useless_tokens) {
            if (token_value == useless) {
                add = false;	
            }
        }

        if (add) {
            Tokens.push_back(add_token(token_value));
        }
    }

    while (Tokens.size() > Common::MAX_TOKENS) {
        for (int i = Tokens.size() - 1; i >= 0; i--) {
            Tokens.erase(Tokens.begin() + i);
            break;
        }
    }
}

Token* Product::add_token(string token_value) {
    // TODO: Use Hash Map instead of vector to store tokens
    Token token(token_value);
    // If the token is already found in the vector of Tokens, increase the freqency and return the token to be added into the Tokens vector of the product
    for (int i = 0; i < Common::token_hashes.size(); i++) {
        if (Common::token_hashes[i] == token.id) {
            Common::all_tokens[i].frequency++;
            return &Common::all_tokens[i];
        }
    }

    token.frequency++;
    Common::token_hashes.push_back(token.id);
    Token* ptr = Common::all_tokens.data();
    Common::all_tokens.push_back(token);
    ptr++;
    return &Common::all_tokens[Common::all_tokens.size() - 1];
}

void Product::generate_token_map() {
    // Generates the Token hash map (called token_map)
    for (int i = 0; i < Tokens.size(); i++) {
        token_map.insert({i, Tokens[i]});
    }
}

void Product::generate_combinations(int k) {
    // Generates up to x choose 7 combinations
    hash<string> str_hash;
    int comb_id;
    int num_tokens = Tokens.size();
    double distance;

    if (k >= 2) {
        vector<Token*> toks(2);
        string sorted_sig = "";
        sorted_sig.reserve(3);
        for (int i = 0; i < num_tokens; i++) {
            for (int j = i + 1; j < num_tokens; j++) {
                toks[0] = Tokens[i];
                toks[1] = Tokens[j];
                sort(toks.begin(), toks.end());

                // The sorted_sig is what is used to hash the combination
                // Essentially, it is each token's id in the sorted tokens appended to a string with spaces between 
                sorted_sig = "";
                sorted_sig += toks[0]->get_id();
                sorted_sig += " ";
                sorted_sig += toks[1]->get_id();

                // The hash for each combo
                comb_id = str_hash(sorted_sig);

                // Compute the Euclidean distance for the combination (UPM[6])
                distance = sqrt(pow(i, 2) + pow(j - 1, 2));

                // Creation of the Combination object
                Combination comb(toks, comb_id, 2);

                // Add it to the overall vecotor of combinations
                Common::all_combinations.push_back(comb);

                // Increase occurance of this particular combination
                Common::lc_avg += 2;


                // The pointer the *points* to the combination in Common::all_combinations
                Combination* comb_ptr = &Common::all_combinations[Common::all_combinations.size() - 1];

                // Add the combination pointer to the Combinations* vector of the product
                Combinations.push_back(add_combination(comb_id, comb_ptr, distance));
            }
        }
    }

    if (k >= 3) {
        vector<Token*> toks(3);
        string sorted_sig = "";
        sorted_sig.reserve(5);
        for (int i = 0; i < num_tokens; i++) {
            for (int j = i + 1; j < num_tokens; j++) {
                for (int l = j + 1; l < num_tokens; l++) {
                    toks[0] = Tokens[i];
                    toks[1] = Tokens[j];
                    toks[2] = Tokens[l];
                    sort(toks.begin(), toks.end());

                    // The sorted_sig is what is used to hash the combination
                    // Essentially, it is each token's id in the sorted tokens appended to a string with spaces between 
                    sorted_sig = "";
                    sorted_sig += toks[0]->get_id();
                    sorted_sig += " ";
                    sorted_sig += toks[1]->get_id();
                    sorted_sig += " ";
                    sorted_sig += toks[2]->get_id();

                    comb_id = str_hash(sorted_sig);

                    // Compute the Euclidean distance for the combination (UPM[6])
                    distance = sqrt(pow(i, 2) + pow(j - 1, 2) + pow(l - 2, 2));

                    Combination comb(toks, comb_id, 3);

                    Common::all_combinations.push_back(comb);
                    
                    // Increase occurance of this particular combination
                    Common::lc_avg += 3;

                    Combination* comb_ptr = &Common::all_combinations[Common::all_combinations.size() - 1];
                    
                    Combinations.push_back(add_combination(comb_id, comb_ptr, distance));
                }
            }
        }
    }

    if (k >= 4) {
        vector<Token*> toks(4);
        string sorted_sig = "";
        sorted_sig.reserve(7);

        for (int i = 0; i < num_tokens; i++) {
            for (int j = i + 1; j < num_tokens; j++) {
                for (int l = j + 1; l < num_tokens; l++) {
                    for (int m = l + 1; m < num_tokens; m++) {
                        toks[0] = Tokens[i];
                        toks[1] = Tokens[j];
                        toks[2] = Tokens[l];
                        toks[3] = Tokens[m];
                        sort(toks.begin(), toks.end());


                        // The sorted_sig is what is used to hash the combination
                        // Essentially, it is each token's id in the sorted tokens appended to a string with spaces between 
                        sorted_sig = "";
                        sorted_sig += toks[0]->get_id();
                        sorted_sig += " ";
                        sorted_sig += toks[1]->get_id();
                        sorted_sig += " ";
                        sorted_sig += toks[2]->get_id();
                        sorted_sig += " ";
                        sorted_sig += toks[3]->get_id();

                        comb_id = str_hash(sorted_sig);

                        // Compute the Euclidean distance for the combination (UPM[6])
                        distance = sqrt(pow(i, 2) + pow(j - 1, 2) + pow(l - 2, 2) + pow(m - 3, 2));

                        Combination comb(toks, comb_id, 4);
                
                        // Increase occurance of this particular combination
                        Common::lc_avg += 4;
                        
                        Common::all_combinations.push_back(comb);
                        Combination* comb_ptr = &Common::all_combinations[Common::all_combinations.size() - 1];
                        Combinations.push_back(add_combination(comb_id, comb_ptr, distance));
                    }
                }
            }
        }
    }

    if (k >= 5) {
        vector<Token*> toks(5);
        string sorted_sig = "";
        sorted_sig.reserve(9);
        for (int i = 0; i < num_tokens; i++) {
            for (int j = i + 1; j < num_tokens; j++) {
                for (int l = j + 1; l < num_tokens; l++) {
                    for (int m = l + 1; m < num_tokens; m++) {
                        for (int n = m + 1; n < num_tokens; n++) {
                            toks[0] = Tokens[i];
                            toks[1] = Tokens[j];
                            toks[2] = Tokens[l];
                            toks[3] = Tokens[m];
                            toks[4] = Tokens[n];

                            sort(toks.begin(), toks.end());
                            

                            // The sorted_sig is what is used to hash the combination
                            // Essentially, it is each token's id in the sorted tokens appended to a string with spaces between 
                            sorted_sig = "";
                            sorted_sig += toks[0]->get_id();
                            sorted_sig += " ";
                            sorted_sig += toks[1]->get_id();
                            sorted_sig += " ";
                            sorted_sig += toks[2]->get_id();
                            sorted_sig += " ";
                            sorted_sig += toks[3]->get_id();
                            sorted_sig += " ";
                            sorted_sig += toks[4]->get_id();

                            comb_id = str_hash(sorted_sig);

                            // Compute the Euclidean distance for the combination (UPM[6])
                            distance = sqrt(pow(i, 2) + pow(j - 1, 2) + pow(l - 2, 2) + pow(m - 3, 2) + pow(n - 4, 2));

                            Combination comb(toks, comb_id, 5);
                            Common::all_combinations.push_back(comb);
                            
                            // Increase occurance of this particular combination
                            Common::lc_avg += 5;
                            
                            Combination* comb_ptr = &Common::all_combinations[Common::all_combinations.size() - 1];
                            Combinations.push_back(add_combination(comb_id, comb_ptr, distance));
                        }
                    }
                }
            }
        }
    }

    if (k >= 6) {
        vector<Token*> toks(6);
        string sorted_sig = "";
        sorted_sig.reserve(11);
        for (int i = 0; i < num_tokens; i++) {
            for (int j = i + 1; j < num_tokens; j++) {
                for (int l = j + 1; l < num_tokens; l++) {
                    for (int m = l + 1; m < num_tokens; m++) {
                        for (int n = m + 1; n < num_tokens; n++) {
                            for (int o = n + 1; o < num_tokens; o++) {
                                toks[0] = Tokens[i];
                                toks[1] = Tokens[j];
                                toks[2] = Tokens[l];
                                toks[3] = Tokens[m];
                                toks[4] = Tokens[n];
                                toks[5] = Tokens[o];

                                sort(toks.begin(), toks.end());

                                // The sorted_sig is what is used to hash the combination
                                // Essentially, it is each token's id in the sorted tokens appended to a string with spaces between 
                                sorted_sig = "";
                                sorted_sig += toks[0]->get_id();
                                sorted_sig += " ";
                                sorted_sig += toks[1]->get_id();
                                sorted_sig += " ";
                                sorted_sig += toks[2]->get_id();
                                sorted_sig += " ";
                                sorted_sig += toks[3]->get_id();
                                sorted_sig += " ";
                                sorted_sig += toks[4]->get_id();
                                sorted_sig += " ";
                                sorted_sig += toks[5]->get_id();

                                comb_id = str_hash(sorted_sig);

                                // Compute the Euclidean distance for the combination (UPM[6])
                                distance = sqrt(pow(i, 2) + pow(j - 1, 2) + pow(l - 2, 2) + pow(m - 3, 2) + pow(n - 4, 2) + pow(o - 5, 2));

                                Combination comb(toks, comb_id, 6);
                                Common::all_combinations.push_back(comb);
                                
                                // Increase occurance of this particular combination
                                Common::lc_avg += 6;

                                Combination* comb_ptr = &Common::all_combinations[Common::all_combinations.size() - 1];
                                Combinations.push_back(add_combination(comb_id, comb_ptr, distance));
                            }
                        }
                    }
                }
            }
        }
    }

    if (k >= 70) {
        vector<Token*> toks(7);
        string sorted_sig = "";
        sorted_sig.reserve(13);
        for (int i = 0; i < num_tokens; i++) {
            for (int j = i + 1; j < num_tokens; j++) {
                for (int l = j + 1; l < num_tokens; l++) {
                    for (int m = l + 1; m < num_tokens; m++) {
                        for (int n = m + 1; n < num_tokens; n++) {
                            for (int o = n + 1; o < num_tokens; o++) {
                                for (int p = o + 1; p < num_tokens; p++) {
                                    toks[0] = Tokens[i];
                                    toks[1] = Tokens[j];
                                    toks[2] = Tokens[l];
                                    toks[3] = Tokens[m];
                                    toks[4] = Tokens[n];
                                    toks[5] = Tokens[o];
                                    toks[6] = Tokens[p];

                                    sort(toks.begin(), toks.end());

                                    // The sorted_sig is what is used to hash the combination
                                    // Essentially, it is each token's id in the sorted tokens appended to a string with spaces between 
                                    sorted_sig = "";
                                    sorted_sig += toks[0]->get_id();
                                    sorted_sig += " ";
                                    sorted_sig += toks[1]->get_id();
                                    sorted_sig += " ";
                                    sorted_sig += toks[2]->get_id();
                                    sorted_sig += " ";
                                    sorted_sig += toks[3]->get_id();
                                    sorted_sig += " ";
                                    sorted_sig += toks[4]->get_id();
                                    sorted_sig += " ";
                                    sorted_sig += toks[5]->get_id();
                                    sorted_sig += " ";
                                    sorted_sig += toks[6]->get_id();

                                    comb_id = str_hash(sorted_sig);

                                    // Compute the Euclidean distance for the combination (UPM[6])
                                    distance = sqrt(pow(i, 2) + pow(j - 1, 2) + pow(l - 2, 2) + pow(m - 3, 2) + pow(n - 4, 2) + pow(o - 5, 2) + pow(p - 6, 2));

                                    Combination comb(toks, comb_id, 7);
                                    Common::all_combinations.push_back(comb);
                                    
                                    // Increase occurance of this particular combination
                                    Common::lc_avg += 7;

                                    Combination* comb_ptr = &Common::all_combinations[Common::all_combinations.size() - 1];
                                    Combinations.push_back(add_combination(comb_id, comb_ptr, distance));
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

Combination* Product::add_combination(int hash, Combination* combo, double distance) {

    // If the hash is not in the map (meaning the combo isn't either); insert it
    if (Common::combo_hash_map.count(hash) == 0) {
        combo->dacc = distance;

        // For each combination in the combo_hash_map, compute the average distance by multiplying it by dividing it by its frequency
        combo->dacc_avg = distance;

        // Insert the new combination into the combination hash map
        Common::combo_hash_map.insert({hash, combo});

        return combo;
    }

    // If the hash is found (meaning the combo as well) increase the frequency
    else {
        Combination* t = Common::combo_hash_map[hash];
        t->frequency++;
        t->dacc += distance;

        // For each combination in the combo_hash_map, compute the average distance by multiplying it by dividing it by its frequency
        t->dacc_avg = t->dacc / t->frequency;
        return t;
    }
}

double Product::yc(Combination* c) {
    double sum = 0;
    for (Token* t : c->tokens) {

        // log(Common::lprod / t->frequency) is 0 when log(1)
        sum += (double) ( (double) log( (double) Common::lprod / t->frequency) ) * (double) Common::semantics_weights[t->semantic] / (double) ( (1 - Common::b) + ( (double) ( (double) (Common::b*Common::K) / (double) Common::lc_avg)));
    }

    // cout << sum << endl;
    return sum;

}

void Product::display() {
    cout << "_____________________ " << vendor << endl;
    cout << high_score << endl;
    for (Token* t : high_combo->tokens) {
        cout << t->value << endl;
    }
}

void Product::cluster_creation() {
    double new_score;
    for (Combination* c : Combinations) {
        // /cout << (double) pow(yc(c), 2) << endl;
        // cout << (double) ( (double) pow(yc(c), 2)) / (double) (A + c->dacc_avg) << endl;
        new_score = (double) ( (double) pow(yc(c), 2) / (double) (Common::A + c->dacc_avg)) * (double) log(c->frequency);
        //cout << new_score << endl;
        if (new_score > high_score) {
            high_score = new_score;
            high_combo = c;
            high_combo_hash = c->id;
            //cout << c->id;
        }		
    }
}
