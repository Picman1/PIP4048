//Inquiry Command
String QPI = "\x51\x50\x49\xBE\xAC\x0D";                           //Device Protocol ID Inquiry
String QID = "\x51\x49\x44\xD6\xAE\x0D";                           //The device serial number inquiry
String QVFW = "\x51\x56\x46\x57\x62\x99\x0D";                      //Main CPU Firmware version inquiry                                         // (VERFW:00074.30
String QVFW2 = "\x51\x56\x46\x57\x32\xC3\xF5\x0D";                 //Another CPU Firmware version inquiry
String QFLAG = "\x51\x46\x4C\x41\x47\x98\x74\x0D";                 //Device flag status inquiry
String QPIGS = "\x51\x50\x49\x47\x53\xB7\xA9\x0D";                 //Device general status parameters inquiry
String QMOD = "\x51\x4D\x4F\x44\x49\xC1\x0D";                      //Device Mode inquiry
String QPIWS = "\x51\x50\x49\x57\x53\xB4\xDA\x0D";                 //Device Warning Status inquiry                                              // (00000000000000000000000000000000
String QDI = "\x51\x44\x49\x71\x1B\x0D";                           //The default setting value information
String QMCHGCR = "\x51\x4D\x43\x48\x47\x43\x52\xD8\x55\x0D";       //Enquiry selectable value about max charging current
String QMUCHGCR = "\x51\x4D\x55\x43\x48\x47\x43\x52\x26\x34\x0D";  //Enquiry selectable value about max utility charging current
String QBOOT = "\x51\x42\x4F\x4F\x54\x0A\x88\x0D";                 //Enquiry DSP has bootstrap or not
String QOPM = "\x51\x4F\x50\x4D\xA5\xC5\x0D";                      //Enquiry output mode (For 4000/5000)
String QPIRI = "\x51\x50\x49\x52\x49\xF8\x54\x0D";                 //Device Rating Information inquiry

//Setting parameters Command
String QBV = "\x51\x42\x56\x38\x63\x0D";
String PF = "\x50\x46\x26\xBD\x0D";                                    //Setting control parameter to default value
String POP02 = "\x50\x4F\x50\x30\x32\xE2\x0A\x0D";                     //02 for SBU priority
String POP01 = "\x50\x4F\x50\x30\x31\xD2\x69\x0D";                     //01 for solar first
String POP00 = "\x50\x4F\x50\x30\x30\xC2\x48\x0D";                     //00 for utility first
String PCP00 = "\x50\x43\x50\x30\x30\x8D\x7A\x0D";                     //Setting device charger priority - Set Charging Source Priority: Utility First
String PCP01 = "\x50\x43\x50\x30\x31\x9D\x5B\x0D";                     //Solar First (The inverter will prioritize charging the batteries from solar power).
String PCP02 = "\x50\x43\x50\x30\x32\xAD\x38\x0D";                     //Solar and Utility (The inverter will use both solar and utility power to charge the batteries, typically prioritizing solar when available, but supplementing with utility).
String MUCHGC002 = "\x4D\x55\x43\x48\x47\x43\x30\x30\x32\xB5\xD1\x0D"; //Set the maximum utility charging current to 2 Amps.
String MUCHGC010 = "\x4D\x55\x43\x48\x47\x43\x30\x31\x30\xA6\xA2\x0D";
String MUCHGC020 = "\x4D\x55\x43\x48\x47\x43\x30\x32\x30\xF3\xF1\x0D";
String MUCHGC030 = "\x4D\x55\x43\x48\x47\x43\x30\x33\x30\xC0\xC0\x0D";
String PPCP000 = "\x50\x50\x43\x50\x30\x30\x30\xE6\xE1\x0D";            //Power Prioritization for Charging and Powering the Load simultaneously.
String PPCP001 = "\x50\x50\x43\x50\x30\x30\x31\xF6\xC0\x0D";
String PPCP002 = "\x50\x50\x43\x50\x30\x30\x32\xC6\xA3\x0D";
String QPIGS2 = "\x51\x50\x49\x47\x53\x32\x68\x2D\x0D";                 //"Query General Status" and provides a broad range of operational parameters from the inverter -  for inverters that have two independent PV input strings (two MPPTs)
String POP03 = "\x50\x4F\x50\x30\x33\xF2\x2B\x0D";                      //Solar and Utility (SUB - Solar, Utility, Battery)

// QPIGS
//(236.8 49.8 230.0 49.8 0598 0584 011 434 53.80 000 100 0033 0011 080.1 54.00 00000 01110110 00 00 00610 110
// Breakdown of the values:
// - 236.8  – Grid voltage (V)
// - 49.8   – Grid frequency (Hz)
// - 230.0  – Output voltage (V)
// - 49.8   – Output frequency (Hz)
// - 0598   – Apparent power (VA)
// - 0584   – Active power (W)
// - 011    – Load percentage (%)
// - 434    – Bus voltage (V)
// - 53.80  – Battery voltage (V)
// - 000    – Battery charging current (A)
// - 100    – Battery capacity (%)
// - 0033   – PV input voltage (V)
// - 0011   – PV input current (A)
// - 080.1  – Battery voltage from SCC (V)
// - 54.00  – Battery discharge current (A)
// - 00000  – Reserved field
// - 01110110 – Status flags (binary-coded)
// - 00     – Reserved field
// - 00     – Reserved field
// - 00610  – Reserved field
// - 110    – CRC checksum

// QPIRI
// (230.0 13.0 230.0 50.0 13.0 3000 3000 24.0 25.0 21.5 27.5 27.5 2 02 010 0 2 3 1 01 0 0 27.0 0 1