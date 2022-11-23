#include "respiration.h"
#include <math.h>

/*Ajout de macros pour representer les coefficients
des fonctions respiration_get_mouvement et respiration_get_presence.
Choisi suite a des tests (Pourra etre modifie sur Max MSP).
*/
#define COEFF_MOUVEMENT 1.25   
#define COEFF_PRESENCE  0.2
#define WINDOW_SIZE      60

#define MOTION_TRESHOLD 3.3
#define PRESENCE_TRESHOLD 0.2

#define DIST_MIN 0.3
#define DIST_MAX 4.0

// Valeurs par defaut
static float coeff_mouv = COEFF_MOUVEMENT;
static float coeff_pres = COEFF_PRESENCE;

// Acces aux parametres dynamiques
void setCoeffMouv(float val) { coeff_mouv = val; }
void setCoeffPres(float val) { coeff_pres = val; }
float getCoeffMouv() { return coeff_mouv; }
float getCoeffPres() { return coeff_pres; } 

respiration_data_t* respiration_init(int sensor_array_size, int resp_buffer_size){
    respiration_data_t* resp_dat = (respiration_data_t*)malloc(sizeof(respiration_data_t));

    resp_dat->resp_buffer = (float*)malloc(resp_buffer_size*sizeof(float));
    resp_dat->resp_buffer_size = resp_buffer_size;

    resp_dat->format_filter = init_filter(LOWPASS, 0.05);
    resp_dat->maxx_filter = init_filter(LOWPASS, 0.01);
    resp_dat->resp_filter = init_filter(LOWPASS, 0.1);
    resp_dat->sumMotion_filter = init_filter(LOWPASS, 0.096);

    resp_dat->filter1_data = init_2d_filter(sensor_array_size, LOWPASS ,0.3);
    resp_dat->filter2_data = init_2d_filter(sensor_array_size, HIGHPASS,0.9);
    resp_dat->filter3_data = init_2d_filter(sensor_array_size, LOWPASS ,0.01);

    return(resp_dat);
}

int respiration_update(float *sensor_array, int sensor_array_size, respiration_data_t* respiration_data){
    float formated_sensor_data[sensor_array_size];
    float formated_sensor_data_filt[sensor_array_size];

    float lowpassed_sensor_data[sensor_array_size];
    float highpassed_sensor_data[sensor_array_size];
    float abs_highpassed_sensor_data[sensor_array_size];

    float presence_indicator_data[sensor_array_size];

    float rolling_averaged_data[sensor_array_size];
    float rolling_averaged_data2[sensor_array_size];

    for(int i = 0; i < sensor_array_size; i++){
        formated_sensor_data[i] = abs(sensor_array[i]-255);
    }

    respiration_data->format_filter->output = 0;
    for(int i = 0; i < sensor_array_size; i++){
        formated_sensor_data_filt[i] = updateFilter(formated_sensor_data[i],respiration_data->format_filter);
    }

    update2dFilter(formated_sensor_data_filt, lowpassed_sensor_data, respiration_data->filter1_data);
 
    update2dFilter(formated_sensor_data_filt, highpassed_sensor_data, respiration_data->filter2_data);

    for(int i = 0; i < sensor_array_size; i++){
        abs_highpassed_sensor_data[i] = abs(highpassed_sensor_data[i]);
    }

    update2dFilter(abs_highpassed_sensor_data, presence_indicator_data, respiration_data->filter3_data);

    //set extremities to 0
    for(int i = 0; i < WINDOW_SIZE/2;i++){
        rolling_averaged_data[i] = 0;
    }
    for(int i =  sensor_array_size - (WINDOW_SIZE/2); i < sensor_array_size;i++){
        rolling_averaged_data[i] = 0;
    }

    //Calc rolling average window
    for(int i = WINDOW_SIZE/2; i < sensor_array_size - (WINDOW_SIZE/2);i++){
        float average = 0;
        for(int j = -WINDOW_SIZE/2; j < WINDOW_SIZE/2; j++){
            average += presence_indicator_data[i+j];
        }
        rolling_averaged_data[i] = average/(float)WINDOW_SIZE;
    }

     for(int i = WINDOW_SIZE/2; i < sensor_array_size - (WINDOW_SIZE/2);i++){
        float average = 0;
        for(int j = -WINDOW_SIZE/2; j < WINDOW_SIZE/2; j++){
            average += rolling_averaged_data[i+j];
        }
        rolling_averaged_data2[i] = average/(float)WINDOW_SIZE;
    }

    int maxx = 0;
    float maxy = 0;
    for(int i = 0; i < sensor_array_size; i++){
        if(rolling_averaged_data2[i] > maxy){
            maxx = i;
            maxy = rolling_averaged_data2[i];
        }
    }

    float filt_maxx = updateFilter((float)maxx,respiration_data->maxx_filter);

    respiration_data->max_index = (int) filt_maxx;
    

    for(int i = 0; i < sensor_array_size; i++){
        sensor_array[i] = rolling_averaged_data2[i];
    }

    float breath = breathing_parser(formated_sensor_data_filt,sensor_array_size,maxx);

    float breath_filt = updateFilter(breath,respiration_data->resp_filter);

    for(int i = respiration_data->resp_buffer_size-1; i > 0 ; i--){
        respiration_data->resp_buffer[i] = respiration_data->resp_buffer[i-1];
    }

    float sumMotion = 0;

    for(int i = 0; i < sensor_array_size; i++){
        sumMotion += abs(highpassed_sensor_data[i]);
    }

    float filt_sumMotion = updateFilter(sumMotion,respiration_data->sumMotion_filter);
    int motion = filt_sumMotion > coeff_mouv;

    int presence = filt_sumMotion > coeff_pres;

    respiration_data->resp_buffer[0] = -1 * breath_filt * ((float)motion -1) * presence;

    respiration_data->mouvement = motion;
    respiration_data->presence = presence;

    float distance = (respiration_data->max_index * (float)(DIST_MAX - DIST_MIN)/sensor_array_size + DIST_MIN);

    printf("Maxx : %d, Motion = %d, Presence = %d, dist = %f\n",respiration_data->max_index,motion, presence, distance);

    return(EXIT_SUCCESS);
}

    /*
    %Additionner les points se retrouvant à droite de la plage
    %d'échantillonnage et le mettre dans la variable moyenne
    for i=WINDOW_SIZE/2:1:frameSize-WINDOW_SIZE/2
        moyenne(i) = sum(filtFrame(3,((i-WINDOW_SIZE/2)+1:(i+WINDOW_SIZE/2))));
    end
    
    %Trouver le maximum en x et en y des points et y ajouter la moitié
    %restante pour positionner le point au plus récent
    [maxy maxx] = max(moyenne);
    %maxx = maxx + WINDOW_SIZE / 2;
    */

//Breathing parser function. 
float breathing_parser(float *format_sensor_array, int size_array, int position){
    //Sert a connaitre la moyenne des valeurs
    float moyenne; 
    float somme;

    //Verifier que les valeurs envoye par la fonction n'excedent pas le array
    if(position+(WINDOW_SIZE/2) > size_array ){
        printf("breathing_parser: position + WINDOW_SIZE/2 excede le array");
        return 0;
    }

    if(position-(WINDOW_SIZE/2) < 0) {
        printf("breathing_parser: position - WINDOW_SIZE/2 excede le array");
        return 0;
    }

    if(position < 0) {
        printf("breathing_parser: position ne peut etre plus petit que 0!");
        return 0;
    }
    

    //Faire la somme des valeurs obtenue pour ensuite s'occuper du projet
    for(int i = position-WINDOW_SIZE/2; i<position+WINDOW_SIZE/2; i++){
        somme += format_sensor_array[i];
    }
    
    //
    moyenne = somme/WINDOW_SIZE;

    //Retour de la moyenne
    return moyenne;
}


//Indicator to see if there's movement
//breath_array: valeurs obtenues du capteur
//num_value: nombre de donnees a prendre dans le buffer pour reference (sur matlab = 3)
//output: 1 ou 0 si mouvement ou non
int respiration_get_mouvement(float *breath_array, int num_value){
    int mobilite;
    float sum;
    float means_ref;
    int i;

    //Faire la somme des num_value dernieres valeurs
    for(i=0; i<num_value; i++){
        
        sum += breath_array[i];
    }

    /*faire la moyenne des num_value dernieres valeurs puis diviser
    la premiere valeur entree par la moyenne*/
    means_ref = breath_array[0]/(sum/i);
    
    /*Si la valeur est au dessus de coeff_mouv (arbitraire), alors
    mettre mobilite a 1, sinon 0*/
    if(means_ref > coeff_mouv){
        mobilite = 1; 
    }
    else{
        mobilite = 0;
    }

    return mobilite;
}

/*  Indicator to see if there's presence
    breath_array: valeurs obtenues du capteur
    num_value: nombre de valeurs a analyser dans le projet
    output: 1 ou 0 si presence ou non
*/
int respiration_get_presence(float *breath_array, int num_value){
    //Pour calculer la moyenne des valeurs recentes (0 a num_value/2)
    float sum_past = 0.00;
    float sum_recent = 0.00;
    int presence;

    //Pour calculer la moyenne des valeurs passees (num_value/2 a num_value)
    float means_recent;
    float means_past;
    int i;
    int j;
    int valeur = num_value;

    //Tableau temporaire pour calcul du coefficient de reference
    float tab_analysis[valeur];

    //Faire la somme des num_value dernieres valeurs
    for (i = 0; i < (num_value / 2); i++) {
        printf("sum_past: %f\n", sum_past);

        //-1 puisque la premiere valeur debute a la position 0
        printf("breath_array: %f\n\n", breath_array[num_value-i-1]);
        sum_past += breath_array[num_value - i - 1];
        
        //A utiliser au besoin pour un autre test
        //sum_recent += breath_array[i];
    }
    
    //Calcul de la moyenne des derniers points
    means_past = sum_past / (num_value / 2);

    //Soustraire chacune des valeurs recentes par la moyenne des derniers points
    for (j = 0; j < (num_value / 2); j++) {
        tab_analysis[j] = breath_array[j] - means_past;
        printf("\ntab_analysis: %f \n", tab_analysis[j]);
    }

    /*Faire la moyenne des valeurs obtenus puis faire
    l'absolue pour que la difference soit positif*/
    for (j = 0; j < (num_value / 2); j++) {
        sum_recent += tab_analysis[j];
    }
    means_recent = abs(sum_recent / (num_value / 2));

    /*Si cette valeur est sous COEFF_PRESENCE (arbitraire), alors il n'y a personne
    Sinon, presence = 1*/
    if (means_recent < coeff_pres) {
        presence = 0;
    }

    else {
        presence = 1;
    }

    return presence;
}
