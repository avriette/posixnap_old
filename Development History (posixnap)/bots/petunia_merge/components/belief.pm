
sub public {
	my ($thischan, $thisuser, $thismsg) = @_;
	return unless $thismsg =~ /^:belief/;
	utility::spew( $thischan, $thisuser, genbelief() );
}

sub private {
	my ($thischan, $thisuser, $thismsg) = @_;
	return unless $thismsg =~ /^:belief/;
	utility::spew( $thischan, $thisuser, genbelief() );
}

sub help {
	my ($thischan, $thisuser, $thismsg) = @_;
	utility::spew( $thischan, $thisuser, 
	  "Type :belief to get religion." );
}

sub genbelief {
	my %statements = ('religion' => [ "Aryan",
              			  "Athanasian",
              		          "Roman",
              			  "Sunni",
              			  "Orthogonal",
              			  "Presbyterian",
              			  "United",
              			  "Reformed",
              			  "Unitarian",
              			  "Anglican",
              			  "Jehovah\'s",
              			  "Jewish",
              			  "Ancient",
              			  "Fundamentalist",
              			  "Shiite",
              			  "Hari",
              			  "Freudian",
              			  "Hegelian",
              			  "Shinto",
              			  "Zoroastrian",
              			  "Christian",
              			  "Salvation",
              			  "Pandimensional",
              			  "Seventh Day",
              			  "Saphardic",
              			  "Zen",
              			  "Hebraic",
              			  "Ye Olde",
              			  "Palestinian",
              			  "African",
              			  "Polynesian",
              			  "Galilean",
              			  "New Age",
              			  "Discordian",
              			  "Reverted",
              			  "Reformed",
              			  "Orthodox",
              			  "Synaptic",
              			  "Cranial",
              			  "Southern",
              			  "Northern",
              			  "Eastern",
              			  "Western"], 

		 'qualifiers' => ["Catholic",
               			  "Baptist",
               			  "Protestant",
               			  "Muslim",
               			  "Moonie",
               			  "Mormon",
               			  "Illuminati",
               			  "Witnesses",
               			  "Sikh",
               			  "Buddhist",
               			  "Krishna",
               			  "Methodist",
               			  "Fire-worshipper",
               			  "Parsee",
               			  "Theosophist",
               			  "Science",
               			  "Army",
               			  "Adventist",
               			  "Confucianist",
               			  "Shamanist",
               			  "Taoist",
               			  "Atheist",
               			  "Rosicrusian",
               			  "Sub-Genius",
               			  "Episcopalian",
               			  "Secular Humanist",
               			  "Revisionist",
               			  "Masonic",
               			  "Pentecostal",
               			  "Charismatic",
               			  "J.F.K.",
               			  "Thetan",
               			  "Odd Fellow",
               			  "PreClear",
               			  "Twelve Step",
               			  "Druid",
               			  "Demolition",
               			  "Lutheran",
               			  "Four Square",
               			  "Bible Thumpers",
               			  "Flat-Earth",
               			  "Whole-Earth"],
	      'institutions'  => ["Church",
                  		  "Faith",
                  		  "Sect",
                  		  "Heresy",
                  		  "Liberation Front",
                  		  "Elder Council",
                  		  "Lodge",
                  		  "Society",
                  		  "Temple",
                  		  "Army",
                  		  "Congregation",
                  		  "Sunday School",
                  		  "Cult",
                  		  "Institute",
                  		  "Gang",
                  		  "Squad",
                  		  "Shrine",
                  		  "Mothers",
                  		  "Fathers",
                  		  "Sisters",
                  		  "Fellowship",
                  		  "Office",
                  		  "Brothers"],
		'speakverbs'  => ["believes that",
                		  "denies that",
                		  "is strongly divided over whether",
                		  "rejects the notion that",
                		  "posits that",
                		  "insists that",
                		  "vows to convert all who believe that",
                		  "professes that",
                		  "proclaims that",
                		  "suspects that",
                		  "maintains that",
                		  "holds that",
              		  	  "is not at all certain that",
                		  "disseminates rumors that",
                		  "has announced that"],

 	    'holyscriptures'  => ["the Bible",
                    		  "the Koran",
                    		  "the Magna Carta",
                    		  "the Kama Sutra",
                    		  "the Bagavad Gita",
                    		  "the Talmud",
                    		  "the Torah",
                    		  "the Satanic Verses",
                    		  "the Mosaic Law",
                    		  "the book of Revelation",
                    		  "Jimmie Hendrix\'s \"Purple Haze\"",
                    		  "the inscription on the handle of Dirty Harry\'s Magnum",
                    	 	  "the collection of predictions by Nostradamus",
                    		  "Edgar Allan Poe\'s story \"The Fall of the House of Usher\"",
                    		  "the pattern of wrinkles on O.J. Simpson\'s glove",
                     		  "every Norse saga",
                    		  "the new, controversial eleventh Commandment",
                    		  "the Apocrypha",
                    		  "the Sargent Pepper album",
                    		  "the Athanasian creed",
                    		  "the Rosetta Stone",
                    		  "chapter thirteen of Dianetics",
                    		  "a transcript of the very first Ramtha channeling",
                    		  "The Writ of Common Wisdom",
                    		  "a certain part of \"The Last Temptation of Christ\"",
                    		  "\"The Prophet\", by Kahlil Gibran",
                    		  "\"The Profit\", by Kehlog Albran",
                    		  "a secret document hidden in Fawn Hall\'s underwear",
                    		  "one of the lesser known Dead Sea Scrolls",
                    		  "the Marquis de Sade\'s \"Bedroom Etiquette\"",
                    		  "a cryptic notation on the back of the bar tab from the Last Supper",
                    		  "Darwin\'s \"Origin of Species\'",
                    		  "a particularly crude example of poetry from the men\'s room wall",
                    		  "Elvis\'s Will",
                    		  "Encyclopaedia Brittanica entry on \"Frankenstein, Dr\"",
                    		  "The Pentagon Papers",
                    		  "\"The Pickwick Papers\'",
                    		  "Shirley McLaine\'s \"Out on a Limb\"",
                    		  "an inscription found on the basement wall in the Temple of Karnak",
                    		  "\"The Life of Brian\"",
                    		  "a certain scrap of Algol source code from 1971",
                    		  "an IBM punch card found on the machine room floor in 1978, in reality",
                    		  "the private diary of Marcia Brady",
                    	 	  "the Book of Mormon (with introduction by Charlton Heston),",
                    		  "grafitti once found on the A-train to Far Rockaway in 1969",
                    		  "a certain limerick containing a reference to \"Nantucket\", in reality",
                    		  "Randy Travis\' Copenhagen ring",
                    		  "a mysterious pattern of cat box scratchings",
                    		  "\"The Yosemite\" by John Muir",
                    		  "a certain Windows 95 error message",
                    		  "the Cthulhu mythos",
                    		  "\"Monty Python\'s Flying Circus\"",
                    		  "\"No Bad Dogs\', by Barbara Woodhouse",
                    		  "\"Atlas Shrugged\", by Ayn Rand",
                    		  "a certain cryptic coding construct of Richard Stallman",
                    		  "Cher\'s secret tattoo",
                    		  "the bumps on the skull of Thomas \"Tip\" O\"Neill (interpreted as braille)",
                    		  "Jonathan Livingston Seagull",
                    		  "\"The Joy of Sex\"",
                    		  "\"Primary Colors\' by Anonymous",
                    		  "the embroidery on Newt Gingrich\'s handkerchief",
                    		  "a particularly obscure pattern of crop circles",
                    		  "scene II, act V of \"The Search for Spock\"",
                    		  "a forgotten screenplay by Sidney Sheldon",
                    		  "a certain old \"Leave it to Beaver\" script",
                    		  "Nancy Reagan\'s \"My Turn\""],

		    'rumors'  => ["predicts the eventual supremacy of",
            			  "contains a direct reference to",
            			  "has been misunderstood by",
            			  "proclaims the divinity of",
            			  "can be read as denouncing the divinity of",
            			  "is an insult to",
            			  "is a heretical misinterpretation of scripture by",
            		  	  "was inspired by",
            			  "denies the existence of",
            			  "contains coded messages from the Inner Child of",
            			  "was dictated by",
            			  "encodes the personal opinions of",
            			  "can be deduced from the writings of",
            			  "denies the existence of",
            			  "implies the divinity of",
            			  "justifies worshipping the sandals of",
            			  "explains the holy writings of",
            			  "can be interpreted as the memoirs of",
            			  "if read backwards, reveals cryptic messages from",
            			  "if held up to strong light, reveals a message from"],

		    'people'  => ["Rush Limbaugh",
				  "Salman Rushdie,",
				  "Ringo Starr,",
				  "the Ayatollah,",
				  "Donovan",
				  "Thomas Aquinas,",
				  "Catherine MacKinnon,",
				  "Billy Graham,",
				  "J. Z. Knight,",
				  "Elijah,",
				  "Jim Morrison",
				  "Elvis,",
				  "Elvis Costello,",
				  "Elvira, Mistress of the Dark,",
				  "Jesus\'s little known brother Tony,",
				  "the Reverend Al Sharpton,",
				  "Louisiana State Senator Merle Jacobs,",
				  "John Lennon,",
				  "Buddy Holly,",
				  "Andrew Lloyd Weber,",
				  "a certain bombastic revivalist preacher,",
				  "the creature from the Black Lagoon,",
				  "a particular White-Supremacist from Hayden Lake,",
				  "a certain group of smut-fighting senators",
				  "Kazak the dog",
				  "Jerry Lee Lewis,",
				  "a twelve year old stigmatic from Lubbock Texas,",
				  "Zeus,",
				  "David Lynch,",
				  "King Tut,",
				  "Ernest Hemingway,",
				  "Muriel Hemingway,",
				  "Marilyn Monroe",
			  	  "\"Jake the Snake\",",
				  "Bess Truman,",
				  "a fire-and-brimstone street preacher from Provo,",
				  "the last living Baghwan Shree-Rajneesh loyalist,",
				  "a certain white-shoed car salesman,",
				  "Crazy Eddie,",
				  "Howard Stern,",
				  "a certain Australian footballer named Bubba,",
				  "Father Guido Sarducci,",
				  "the last practicing medicine man of the Mohicans,",
				  "James Randi,",
				  "Our Lady of the White Go-Go Boot,",
				  "Verleen, the patron saint of Big Hair,",
				  "Helga, the patron saint of Unseasoned Food,",
				  "Randy, the patron saint of Big Pickup Trucks,",
				  "Maxine, the patron saint of Press-on Nails,",
				  "Tina, the patron saint of Polyester Stretch Pants,",
				  "Boopsie, the patron saint of Large Dangly Earrings,",
				  "Mahomet,",
				  "Walt Disney,",
				  "Ezekiel,",
				  "Dan Quayle,",
				  "Murphy Brown,",
				  "George Bush,",
				  "Oliver North,",
				  "Thomas Merton",
				  "Leo Buscaglia,",
				  "Marilyn Quayle,",
				  "Ernest Angsley,",
				  "Gautama Buddha,",
				  "Odin and Thor,",
				  "Adolf Hitler,",
				  "Linus Pauling,",
				  "Pope John-Paul II,",
				  "Bishop Spong,",
				  "Mumon,",
				  "Martin Scorcese,",
				  "Bill Gates,",
				  "Daryl Gates,",
				  "Robert Gates,",
				  "Robert DeNiro",
				  "the Mormon Tabernacle Choir,",
				  "Jim Bakker,",
				  "Frank Zappa",
				  "Tammy-Faye Bakker,",
				  "Jimmy Swaggart,",
				  "Susanne Somers,",
				  "Richard Simmons,",
				  "Richard Lewis,",
				  "Cardinal Richelieu,",
				  "Charles Manson,",
				  "James Taylor,",
				  "Tipper Gore,",
				  "Hillary Clinton,",
				  "Dolly Parton,",
				  "Jerry Falwell,",
				  "Robert Bly,",
				  "Joseph Campbell,",
				  "Zig Zeigler,",
				  "L. Ron Hubbard,",
				  "Captain Al Hubbard,",
				  "Jesse Helms,",
				  "Jesse Jackson,",
				  "Jimi Hendrix,",
				  "Jimmy Swaggart,",
				  "The Juice Man (tm),",
				  "Clarence Thomas,",
				  "Long Dong Silver,",
				  "Richard Pryor,",
				  "Britney Spears,",
				  "Jennifer Lopez,",
				  "Ben Affleck,",
				  "Nicholas Cage,",
				  "Pink,",
				  "Heather Grahm,",
				  "Alex Avriette,",
				  "Tyler Hardison,",
				  "Ron Howard,",
				  "Cormac Mannion,",
				  "Nicholas Negroponte",
				  "a certain paunchy, ex-Chippendale dancer,",
				  "the shadowy leader of a certain gang of biker nuns,",
				  "a spaced out, pot smoking crystal worshipper,",
				  "a certain swarthy wild-eyed, Eastern European anarchist,",
				  "a certain middle-aged bag lady with a personality disorder,",
				  "a certain pornography-addicted TV preacher,",
				  "a certain philandering, saxophone-playing Southern Governor,",
				  "a certain flamboyant, gender-bending rock star,",
				  "a certain female talk show pop-psychologist,",
				  "a certain truck stop waitress from Cocolalla, Idaho,",
				  "a certain small town sheriff from North Carolina,",
				  "an avowed heterosexual female tennis player,",
				  "a certain absent-minded, father figure ex-President,",
				  "a certain blonde, pointy-bra wearing pop singer,",
				  "David Duke,",
				  "Robert Mapplethorpe,",
				  "Wilheim Reich,",
				  "Kreskin,",
				  "Mahatma Gandhi,",
				  "Abbie Hoffman,",
				  "Timothy Leary,",
				  "Jack Kerouac,",
				  "Mr Tomaszczyk,",
				  "Allen Ginsberg",
				  "Paul Krassner,",
				  "Marlin Fitzwater,",
				  "Ken Kesey,",
				  "Helen Gurley Brown",
				  "Tina Brown",
				  "David Geffen",
				  "Howard Cosell,",
				  "Sharon Stone",
				  "Cy Sperling,",
				  "Andy Warhol",
				  "Ferdie \"The Fight Doctor\" Pacheco,",
				  "Madonna,",
				  "ex-Polish president Lech Walesa,",
				  "MarYan,",
				  "Ted Kaczynski,",
				  "The Beatles,"],

	       'conjunctions' => ["and",
              			  "but"],

	         'alsoverbs'  => [
				  "says",
				  "explains",
				  "claims",
				  "believes",
				  "argues",
				  "insists",
				  "refutes",
				  "denies",
				  "professes",
				  "declares",
				  "swears",
				  "secretly teaches",
				  "preaches"],

		     'thats'  => [
				  "that",
				  "that soon",
				  "that surely",
				  "that, like it or not,",
				  "that always during the full moon,",
				  "that after the second coming",
				  "that ultimately",
				  "that inevitably",
				  "that unfortunately,",
				  "that in the year 1998,",
				  "that if we are true and faithful servants,"],

	       'predictions'  => [
				  "we should strive for",
				  "the world will end with",
				  "we should beware",
				  "we shall see",
				  "the ungodly are about to experience",
				  "only the faithful will achieve",
				  "the chosen people will experience",
				  "no-one will be allowed to ignore",
				  "we will no more suffer",
				  "men will yield to",
				  "women will yield to",
				  "mankind will yield to",
				  "man will succeed in avoiding",
				  "woman will succeed in avoiding",
				  "mankind will succeed in avoiding",
				  "only the born-again will actually enjoy",
				  "only the chosen people will experience",
				  "Kikuyu tribes will rediscover",
				  "we will be haunted by the specter of",
				  "the government will attempt to penalize any mention of",
				  "unspeakable peril will befall",
				  "the communists will decry",
				  "the faithless will reject",
				  "the corporate business will be victimized by the prophets of",
				  "weather anomalies will jeopardize",
				  "the chosen people will be spared"],
			
		  'outcomes'  => [
				  "reincarnation.",
				  "Judgement Day.",
				  "the Apocalypse.",
				  "\"Apocalypse Now\".",
				  "Oral Robert\'s prayer tower.",
				  "nirvana.",
				  "Nirvana (the band).",
				  "Mount Olympus.",
				  "martyrdom.",
				  "the Second Coming.",
				  "the sound of one hand clapping.",
				  "birth control.",
				  "better living through chemistry.",
				  "human sacrifices.",
				  "cherry pie and damn fine coffee.",
				  "damnation.",
				  "Papal Inphallibility.",
				  "enlightenment.",
				  "the \"666\" mysteriously found tattooed on your head.",
				  "pinball heaven.",
				  "phallic idolatry.",
				  "Coke.",
				  "a guilt-free afterlife.",
				  "guilt-free sex.",
				  "guilt-free bingo.",
				  "anarchy.",
				  "government, by the people, and for the people.",
				  "remorse for their crimes.",
				  "\"Crimes Against Nature\" (at least in some Southern states).",
				  "the Turin Shroud.",
				  "purgatory.",
				  "premarital sex.",
				  "a feeling of oneness with nature.",
				  "that oh-so-fresh feeling.",
				  "that not-so-fresh feeling.",
				  "shiatsu massage.",
				  "chiropractic treatment.",
				  "ESO.",
				  "tetrahydrocannabinol.",
				  "lysergic acid diethylamide.",
				  "psilocybin.",
				  "quinuclidinyl benzilate.",
				  "phenylpropanolamine.",
				  "Xanax addiction.",
				  "Prozac addiction.",
				  "a life made livable with clozapine.",
				  "colonic irrigation, twice a day.",
				  "self abuse.",
				  "real tears from Michelangelo\'s \"Pieta\".",
				  "the face of Jesus in a bowl of spaghetti.",
				  "one man, one vote.",
				  "a resurgence of tribbles.",
				  "a Popeil pocket fisherman.",
				  "\"from each according to their ability, to each according to their need\".",
				  "the upcoming Vernal Equinox.",
				  "Buddha Nature.",
				  "the upcoming Summer Solstice.",
			  	  "trout fishing.",
				  "rolfing.",
				  "engrams.",
				  "pampers.",
				  "utopia.",
				  "the Puppet Master.",
				  "the truths anyone may hold to be self-evident.",
				  "bliss.",
				  "the monolith in 2001 A Space Odessy.",
				  "the writings of Douglas Hofstadter.",
				  "the end of the world.",
				  "the eventual domination of world.",
				  "backward masking.",
				  "megadoses of vitamin C."] );

	my $say = "\0";

	map { $say = $say ." " . $statements{$_}[int(rand()*scalar(@{$statements{$_}}))] } 
		( 'religion', 
		  'qualifiers', 
		  'institutions',
		  'speakverbs',
		  'holyscriptures',
		  'rumors',
		  'people',
		  'conjunctions',
		  'alsoverbs',
		  'thats',
		  'predictions',
		  'outcomes' );

	return $say;
}
