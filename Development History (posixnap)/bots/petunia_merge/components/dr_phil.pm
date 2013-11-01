use warnings;
use strict;

sub public {
	my ($thischan, $thisuser, $thismsg) = (@_);
	utility::spew( $thischan, $thisuser, DrPhil() ) if $thismsg =~ /^:dr_?phil/;
}

sub private {
	my ($thischan, $thisuser, $thismsg) = (@_);
	utility::spew( $thischan, $thisuser, DrPhil() ) if $thismsg =~ /^:dr_?phil/;
}

sub DrPhil {
  #  The Random Dr. Phil Quote Generator
  #  Copyright 2002 MangyDog Productions
  #  http://www.MangyDog.com/

	my @word = ();
	$word[0] = "a bear trap";
	$word[1] = "your Uncle Groucho";
	$word[2] = "Zsa Zsa Gabor";
	$word[3] = "phlebitis";
	$word[4] = "a horseshoe collection";
	$word[5] = "bad manners";
	$word[6] = "a spice rack";
	$word[7] = "a Frisbee";
	$word[8] = "Eminem";
	$word[9] = "palsy";
	$word[10] = "a sack of hammers";
	$word[11] = "pepper spray";
	$word[12] = "shit for brains";
	$word[13] = "parmesan cheese";
	$word[14] = "a badger";
	$word[15] = "Luxembourg";
	$word[16] = "endless supplies of avocados";
	$word[17] = "a broken toe";
	$word[18] = "flapjacks";
	$word[19] = "Oprah's permission";
	$word[20] = "Dr. Phil's permission";
	$word[21] = "orange sherbet";
	$word[22] = "leprosy";
	$word[23] = "Shemp";
	$word[24] = "a nagging mother-in-law";
	$word[25] = "lockjaw";
	$word[26] = "macadamia nuts";
	$word[27] = "a sense of moral decency";
	$word[28] = "the clap";
	$word[29] = "gonorrhea";
	$word[30] = "a leisurely demeanor";
	$word[31] = "a porcupine";
	$word[32] = "a mop";
	$word[33] = "broken fingers";
	$word[34] = "the Power of Cheese";
	$word[35] = "a feeble grandma";
	$word[36] = "clam chowder";
	$word[37] = "five dollars";
	$word[38] = "Jesse Perry";
	$word[39] = "Chad Riden";
	$word[40] = "shingles";
	$word[41] = "husky pants";
	$word[42] = "the Big Bang Theory";
	$word[43] = "zinc";
	$word[44] = "a gambling habit";
	$word[45] = "a silly dance";
	$word[46] = "Cat Scratch Fever";
	$word[47] = "formaldehyde";
	$word[48] = "Flopsy, Mopsy, and Cottontail";
	$word[49] = "a new drug";
	$word[50] = "sideburns";
	$word[51] = "Anna Nicole Smith";
	$word[52] = "tight-fitting pants";
	$word[53] = "a wing and a prayer";
	$word[54] = "feelings";
	$word[55] = "to learn the bossa nova";
	$word[56] = "to go to school";
	$word[57] = "handouts";
	$word[58] = "the weight of the world on your shoulders";
	$word[59] = "a big hug";
	$word[60] = "stain-resistant pants";
	$word[61] = "to bitch and moan";
	$word[62] = "tap dancing lessons";
	$word[63] = "a bag of wolverines";
	$word[64] = "a lucky rabbit's foot";
	$word[65] = "to call the Pope";
	$word[66] = "a major in Philosophy";
	$word[67] = "to feel bad";
	$word[68] = "to feel good";
	$word[69] = "athlete's foot";
	$word[70] = "me to tell you";
	$word[71] = "yer Mama";
	$word[72] = "to watch my crappy show";
	$word[73] = "your peeps";
	$word[74] = "homiez";
	$word[75] = "zilch";
	$word[76] = "a rope";
	$word[77] = "to ask permission";
	$word[78] = "brainpower";
	$word[79] = "lead poisoning";
	$word[80] = "anyone or anything";
	$word[81] = "a lesson in astrophysics";
	$word[82] = "a pack of wild horses";
	$word[83] = "to flunk out of school";
	$word[84] = "a diploma";
	$word[85] = "lesions on your knees";
	$word[86] = "an asthma attack";
	$word[87] = "a bad credit rating";
	$word[88] = "to make a big fuss";
	$word[89] = "a Supreme Court decision";
	$word[90] = "a 50-watt light bulb";
	$word[91] = "to think clearly";
	$word[92] = "to call the media in advance";
	$word[93] = "a personal tragedy";
	$word[94] = "an open mind";
	$word[95] = "to take a personal inventory";
	$word[96] = "to send out a press release";
	$word[97] = "to work out";
	$word[98] = "to rob a bank";
	$word[99] = "help";
	$word[100] = "support from your family";
	$word[101] = "to be RuPaul";
	$word[102] = "to be on Jerry Springer";
	$word[103] = "my hot burning love";
	$word[104] = "a giant schlong";
	$word[105] = "an IT department";
	$word[106] = "two turtledoves";
	$word[107] = "three french hens";
	$word[108] = "a subscription to Hustler";
	$word[109] = "an invisible friend";
	$word[110] = "mild schizophrenia";
	$word[111] = "multiple personality disorder";
	my @word2;
	$word2[0] = "kiss a Quaker";
	$word2[1] = "steal somebody's nougat";
	$word2[2] = "make out with somebody's platypus";
	$word2[3] = "learn how to make a sandwich";
	$word2[4] = "fondle a stranger";
	$word2[5] = "pee in a monastery";
	$word2[6] = "expose yourself";
	$word2[7] = "laminate your shirt";
	$word2[8] = "rock me like a hurricane";
	$word2[9] = "learn about proper dental hygiene";
	$word2[10] = "put salt on your Froot Loops";
	$word2[11] = "join the Peace Corps";
	$word2[12] = "go to MangyDog.com";
	$word2[13] = "go to NashvilleStandup.com";
	$word2[14] = "act real sassy-like";
	$word2[15] = "punch a dogie";
	$word2[16] = "cha-cha-cha all night long";
	$word2[17] = "fondle a donkey";
	$word2[18] = "complain about these kids nowadays";
	$word2[19] = "hug somebody you wanna kill";
	$word2[20] = "learn to love";
	$word2[21] = "get all uppity";
  $word2[22] = "root, root, root for the home team";
	$word2[23] = "dance like an idiot 'til yer toes fall off";
	$word2[24] = "do the Hustle";
	$word2[25] = "dare to be stupid";
	$word2[26] = "poop on a cracker";
	$word2[27] = "tell off a loved one";
	$word2[28] = "put some pep in your step";
	$word2[29] = "take a big swim in Lake Jackass";
	$word2[30] = "shock the monkey";
	$word2[31] = "tickle a gypsy";
	$word2[32] = "do the wild thing";
	$word2[33] = "steal your neighbor's porn";
	$word2[34] = "throw a brick at a turtle";
	$word2[35] = "party 'til ya puke";
	$word2[36] = "feel like an asshole";
	$word2[37] = "cry a lot";
	$word2[38] = "eat a bug";
	$word2[39] = "disgrace your family";
	$word2[40] = "blame it on anybody but yourself";
	$word2[41] = "make a delicious fondue";
	$word2[42] = "grab 'em in the biscuits";
	$word2[43] = "be a dick";
	$word2[44] = "steal a car and go to Missouri";
	$word2[45] = "throw up in Tijuana";
	$word2[46] = "pluck your pubes";
	$word2[47] = "jerk off in a karaoke bar";
	$word2[48] = "WEEWEEWEE all the way home";
	$word2[49] = "think for yourself";
	$word2[50] = "ignore me completely";
	$word2[51] = "raise the roof";
	$word2[52] = "run out into traffic";
	$word2[53] = "whistle while you work";
	$word2[54] = "jump in the lake";
	$word2[55] = "punch a stranger";
	$word2[56] = "join the circus";
	$word2[57] = "staple your lips together";
	$word2[58] = "pinch a stranger's butt";
	$word2[59] = "find your inner moron";
	$word2[60] = "get all up in somebody's grill, yo";
	$word2[61] = ". . . Dadgummit, I forgot";
	$word2[62] = "shave a weasel";
	$word2[63] = "fall in love with an asshole";
	$word2[64] = "goof around and do squat";
	$word2[65] = "pop a blister";
	$word2[66] = "save your toenail clippings";
	$word2[67] = "blow my mind";
	$word2[68] = "wash your neck";
	$word2[69] = "let me tell you how to live";
	$word2[70] = "watch Spongebob Squarepants";
	$word2[71] = "get drunk and pass out";
	$word2[72] = "hear me ramble like a boob";
	$word2[73] = "buy my book";
	$word2[74] = "drop and give me twenty";
	$word2[75] = "hate my guts";
	$word2[76] = "wish I would shut up";
	$word2[77] = "enjoy my shiny bald head";
	$word2[78] = "pray I leave TV and never come back";
	$word2[79] = "wax your elbows";
	$word2[80] = "live in a van down by the river";
	$word2[81] = "do jack-squat with yer life";
	$word2[82] = "waste time at the Mangy Forum";
	$word2[83] = "go to the MangyDog.com Video Vault and laugh yer butt off";
	$word2[84] = "buy a shirt at the MangyDog.com Store";
	$word2[85] = "join the Jehovah's Witnesses";
	$word2[86] = "check out the MangyDog.com Column Archives";
	$word2[87] = "join the MangyDog.com mailing list";
	$word2[88] = "be irritating, just like me";
	$word2[89] = "run a sweatshop";
	$word2[90] = "buy a stairway to heaven";
	$word2[91] = "treat yourself to a tasty shake";
	$word2[92] = "go straight to H-E-Double Hockeysticks";
	$word2[93] = "leave your cares behind";
	$word2[94] = "put me in a headlock";
	$word2[95] = "touch yourself and giggle";
	$word2[96] = "squeal like a pig";
	$word2[97] = "slap a cop";
	$word2[98] = "skin a gopher";
	$word2[99] = "headbutt a priest";
	$word2[100] = "live in Alabama";
	$word2[101] = "get breast implants";
	$word2[102] = "defeat an Irken Invader";
	$word2[103] = "be a script kiddie";
	$word2[104] = "be Theo's friend";
	$word2[105] = "be my kinky love slave";
	$word2[106] = "sing hymns naked in the park";
	$word2[107] = "borrow my anal toys";
	$word2[108] = "make a wonderful beef stew";
	$word2[109] = "get oral sex from random people in sex shops";
	$word2[110] = "be invited to my gang bang";
	$word2[111] = "be a model";

	#  choose the random words

	my $the_number = int rand $#word;
	my $the_word = $word[ $the_number ];
	my $the_number2 = int rand $#word2;
	my $the_word2 = $word2[ $the_number2 ];

	#  return the string

	return "You don't need ". $the_word . " to " . $the_word2 . ".";
}
