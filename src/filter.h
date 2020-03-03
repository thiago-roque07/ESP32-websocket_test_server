


/** http://www.iowahills.com/4IIRFilterPage.html **/
/** Notch Filter Coefficients **/
/** Calculated from: Direct-From II, 2nd Order, Fnotch = 60Hz, Fs = 200Hz, Q = 5 **/
static float b0_n = 0.91152070566246357;
static float b1_n = 0.56335077754869112;
static float b2_n = 0.91152070566246357;
static float a1_n = 0.56335077754869112;
static float a2_n = 0.82304141132492714;

/** Hp Filter Coefficients **/
/** Calculated from: Direct-From II, 2nd Order, Fc = 1Hz, Fs = 200Hz **/
static float b0_hp = 0.978030479206559722;
static float b1_hp = -1.956060958413119440;
static float b2_hp = 0.978030479206559722;
static float a1_hp = -1.955578240315035470;
static float a2_hp = 0.956543676511203311;



float filterNotch(float);
float filterHighPass(float);
float readMeasure();
