#include "../main/bot.h"
#include <math.h>
PluginInfo *ph;

#define any_lowercase_letter  case 'a':case 'b':case'c':case'd':case'e':case'f':case 'g':case 'h':case 'i':case 'j':case 'k':case 'l':case'm':case'n':case'o':case'p':case 'q':case 'r':case 's':case 't':case 'u':case 'v':case'w':case'x':case'y':case'z':
#define any_decimal_digit    case '0':case '1':case'2':case'3':case'4':case'5':case '6':case '7': case '8':case '9':

static int LastLearned = -1;
static int LastGM = -1;
static int LastFB = -1;

static char *CreateDataPath2(char *Buffer, const char *Group, const char *Nick, const char *FileType) {
  char NickBuf[512];
  int i=0;
  while(1) {
    NickBuf[i]=tolower(Nick[i]);
    if(NickBuf[i]=='/' || NickBuf[i]=='\\' || NickBuf[i]=='.')
      NickBuf[i] = '-';
    if(0==Nick[i++])
      break;
  }
  sprintf(Buffer, "/home/nova/.xchat2/NovaBot/data/%s%s%s", Group,NickBuf,FileType);
  return(Buffer);
}
static int rand2(int Max) {
  if(Max <= 0) return 0;
  int Mask = 1;
  while(Mask < (Max-1))
    Mask = (Mask<<1) | 1;
  int Try;
  while(1) {
    Try = rand()&Mask;
    if(Try>(Max-1)) continue;
    break;
  }
  return Try;
}
static char *VerbList[] = 
{"abide","accelerate","accept","accomplish","achieve","acquire","acted","activate","adapt","add","address","administer","admire","admit","adopt","advise","afford","agree","alert","alight","allow","altered","amuse","analyze","announce","annoy","answer","anticipate","apologize","appear","applaud","applied","appoint","appraise","appreciate","approve","arbitrate","argue","arise","arrange","arrest","arrive","ascertain","ask","assemble","assess","assist","assure","attach","attack","attain","attempt","attend","attract","audited","avoid","awake","back","bake","balance","ban","bang","bare","bat","bathe","battle","be","beam","bear","beat","become","beg","begin","behave","behold","belong","bend","beset","bet","bid","bind","bite","bleach","bleed","bless","blind","blink","blot","blow","blush","boast","boil","bolt","bomb","book","bore","borrow","bounce","bow","box","brake","branch","break","breathe","breed","brief","bring","broadcast","bruise","brush","bubble","budget","build","bump","burn","burst","bury","bust","buy","buzz","calculate","call","camp","care","carry","carve","cast","catalog","catch","cause","challenge","change","charge","chart","chase","cheat","check","cheer","chew","choke","choose","chop","claim","clap","clarify","classify","clean","clear","cling","clip","close","clothe","coach","coil","collect","color","comb","come","command","communicate",
"compare","compete","compile","complain","complete","compose","compute","conceive","concentrate","conceptualize","concern","conclude","conduct","confess","confront","confuse","connect","conserve","consider","consist","consolidate","construct","consult","contain","continue","contract","control","convert","coordinate","copy","correct","correlate","cost","cough","counsel","count","cover","crack","crash","crawl","create","creep","critique","cross","crush","cry","cure","curl","curve","cut","cycle","dam","damage","dance","dare","deal","decay","deceive","decide","decorate","define","delay","delegate","delight","deliver","demonstrate","depend","describe","desert","deserve","design","destroy","detail","detect","determine","develop","devise","diagnose","dig","direct","disagree","disappear","disapprove","disarm","discover","dislike","dispense","display","disprove","dissect","distribute","dive","divert","divide","do","double","doubt","draft","drag","drain","dramatize","draw","dream","dress","drink","drip","drive","drop","drown","drum","dry","dust","dwell","earn","eat","edited","educate","eliminate","embarrass","employ","empty","enacted","encourage","end","endure","enforce","engineer","enhance","enjoy","enlist","ensure","enter","entertain","escape","establish","estimate","evaluate","examine","exceed","excite","excuse","execute","exercise","exhibit","exist",
"expand","expect","expedite","experiment","explain","explode","express","extend","extract","face","facilitate","fade","fail","fancy","fasten","fax","fear","feed","feel","fence","fetch","fight","file","fill","film","finalize","finance","find","fire","fit","fix","flap","flash","flee","fling","float","flood","flow","flower","fly","fold","follow","fool","forbid","force","forecast","forego","foresee","foretell","forget","forgive","form","formulate","forsake","frame","freeze","frighten","fry","gather","gaze","generate","get","give","glow","glue","go","govern","grab","graduate","grate","grease","greet","grin","grind","grip","groan","grow","guarantee","guard","guess","guide","hammer","hand","handle","handwrite","hang","happen","harass","harm","hate","haunt","head","heal","heap","hear","heat","help","hide","hit","hold","hook","hop","hope","hover","hug","hum","hunt","hurry","hurt","hypothesize","identify","ignore","illustrate","imagine","implement","impress","improve","improvise","include","increase","induce","influence","inform","initiate","inject","injure","inlay","innovate","input","inspect","inspire","install","institute","instruct","insure","integrate","intend","intensify","interest","interfere","interlay","interpret","interrupt","interview","introduce","invent","inventory","investigate","invite","irritate","itch","jail","jam","jog","join","joke",
"judge","juggle","jump","justify","keep","kept","kick","kill","kiss","kneel","knit","knock","knot","know","label","land","last","laugh","launch","lay","lead","lean","leap","learn","leave","lecture","led","lend","let","level","license","lick","lie","lifted","light","lighten","like","list","listen","live","load","locate","lock","log","long","look","lose","love","maintain","make","man","manage","manipulate","manufacture","map","march","mark","market","marry","match","mate","matter","mean","measure","meddle","mediate","meet","melt","melt","memorize","mend","mentor","milk","mine","mislead","miss","misspell","mistake","misunderstand","mix","moan","model","modify","monitor","moor","motivate","mourn","move","mow","muddle","mug","multiply","murder","nail","name","navigate","need","negotiate","nest","nod","nominate","normalize","note","notice","number","obey","object","observe","obtain","occur","offend","offer","officiate","open","operate","order","organize","oriented","originate","overcome","overdo","overdraw","overflow","overhear",
"overtake","overthrow","owe","own","pack","paddle","paint","park","part","participate","pass","paste","pat","pause","pay","peck","pedal","peel","peep","perceive","perfect","perform","permit","persuade","phone","photograph","pick","pilot","pinch","pine","pinpoint","pioneer","place","plan","plant","play","plead","please","plug","point","poke","polish","pop","possess","post","pour","practice","praised","pray","preach","precede","predict","prefer","prepare","prescribe","present","preserve","preset","preside","press","pretend","prevent","prick","print","process","procure","produce","profess","program","progress","project","promise","promote","proofread","propose","protect","prove","provide","publicize","pull","pump","punch","puncture","punish","purchase","push","put","qualify","question","queue","quit","race","radiate","rain","raise","rank","rate","reach","read","realign","realize","reason","receive","recognize","recommend","reconcile","record","recruit","reduce","refer","reflect","refuse","regret","regulate","rehabilitate","reign",
"reinforce","reject","rejoice","relate","relax","release","rely","remain","remember","remind","remove","render","reorganize","repair","repeat","replace","reply","report","represent","reproduce","request","rescue","research","resolve","respond","restored","restructure","retire","retrieve","return","review","revise","rhyme","rid","ride","ring","rinse","rise","risk","rob","rock","roll","rot","rub","ruin","rule","run","rush","sack","sail","satisfy","save","saw","say","scare","scatter","schedule","scold","scorch","scrape","scratch","scream","screw","scribble","scrub","seal","search","secure","see","seek","select","sell","send","sense","separate","serve","service","set","settle","sew","shade","shake","shape","share","shave","shear","shed","shelter","shine","shiver","shock","shoe","shoot","shop","show","shrink","shrug","shut","sigh","sign","signal","simplify","sin","sing","sink","sip","sit","sketch","ski","skip","slap","slay","sleep","slide","sling","slink","slip","slit","slow","smash","smell","smile","smite","smoke","snatch","sneak","sneeze","sniff","snore","snow","soak","solve",
"soothe","soothsay","sort","sound","sow","spare","spark","sparkle","speak","specify","speed","spell","spend","spill","spin","spit","split","spoil","spot","spray","spread","spring","sprout","squash","squeak","squeal","squeeze","stain","stamp","stand","stare","start","stay","steal","steer","step","stick","stimulate","sting","stink","stir","stitch","stop","store","strap","streamline","strengthen","stretch","stride","strike","string","strip","strive","stroke","structure","study","stuff","sublet","subtract","succeed","suck","suffer","suggest","suit","summarize","supervise","supply","support","suppose","surprise","surround","suspect","suspend","swear","sweat","sweep","swell","swim","swing","switch","symbolize","synthesize","systemize","tabulate","take","talk","tame","tap","target","taste","teach","tear","tease","telephone","tell","tempt","terrify","test","thank","thaw","think","thrive","throw","thrust","tick","tickle","tie","time","tip","tire","touch","tour","tow","trace","trade","train","transcribe","transfer","transform","translate","transport","trap","travel","tread","treat","tremble","trick","trip","trot","trouble","troubleshoot","trust","try","tug","tumble","turn","tutor","twist",
"type","undergo","understand","undertake","undress","unfasten","unify","unite","unlock","unpack","untidy","update","upgrade","uphold","upset","use","utilize","vanish","verbalize","verify","vex","visit","wail","wait","wake","walk","wander","want","warm","warn","wash","waste","watch","water","wave","wear","weave","wed","weep","weigh","welcome","wend","wet","whine","whip","whirl","whisper","whistle","win","wind","wink","wipe","wish","withdraw","withhold","withstand","wobble","wonder","work","worry","wrap","wreck","wrestle","wriggle","wring","write","x-ray","yawn","yell","zip","zoom",NULL};
static char *NounList[] = {"ball","bat","bed","book","boy","bun","can","cake","cap","car","cat","cow","cub","cup","dad","day","dog","doll","dust","fan","feet","girl","gun","hall","hat","hen","jar","kite","man","map","men","mom","pan","pet","pie","pig","pot","rat","son","sun","toe","tub","van","apple","arm","banana","bike","bird","book","chin","clam","class","clover","club","corn","crayon","crow","crown","crowd","crib","desk","dime","dirt","dress","fang","field","flag","flower","fog","game","heat","hill","home","horn","hose","joke","juice","kite","lake","maid","mask","mice","milk","mint","meal","meat","moon","mother","morning","name","nest","nose","pear","pen","pencil","plant","rain","river","road","rock","room","rose","seed","shape","shoe","shop","show","sink","snail","snake","snow","soda","sofa","star","step","stew","stove","straw","string","summer","swing","table","tank","team","tent","test","toes","tree","vest","water","wing","winter","alarm","animal","aunt","bait","balloon","bath","bead","beam","bean","bedroom","boot","bread","brick","brother","camp","chicken","children","crook","deer","dock","doctor","downtown","drum","dust","eye","family","father","fight","flesh","food","frog","goose","grade",
"grandfather","grandmother","grape","grass","hook","horse","jail","jam","kiss","kitten","light","loaf","lock","lunch","lunchroom","meal","mother","notebook","owl","pail","parent","park","plot","rabbit","rake","robin","sack","sail","scale","sea","sister","soap","song","spark","space","spoon","spot","spy","summer","tiger","toad","town","trail","tramp","tray","trick","trip","uncle","vase","winter","water","week","wheel","wish","wool","yard","zebra","actor","airplane","airport","army","baseball","beef","birthday","boy","brush","bushes","butter ","cast","cave","cent","cherries","cherry","cobweb","coil","cracker","dinner","eggnog","elbow","face","fireman","flavor","gate","glove","glue","goldfish","goose","grain","hair","haircut","hobbies","holiday","hot","jellyfish","ladybug","mailbox","number","oatmeal","pail","pancake","pear","pest","popcorn","queen","quicksand","quiet","quilt","rainstorm","scarecrow","scarf","stream","street","sugar","throne","toothpaste","twig","volleyball","wood","wrench","advice","anger","answer","apple","arithmetic","badge","basket","basketball","battle","beast","beetle","beggar","brain","branch","bubble","bucket","cactus","cannon","cattle","celery","cellar","cloth","coach","coast","crate","cream","daughter","donkey","drug","earthquake","feast","fifth","finger","flock","frame","furniture","geese","ghost","giraffe","governor","honey","hope","hydrant","icicle","income","island","jeans","judge","lace","lamp","lettuce","marble","month","north","ocean","patch","plane","playground","poison","riddle","rifle","scale","seashore","sheet","sidewalk","skate","slave","sleet","smoke","stage","station","thrill","throat","throne","title","toothbrush","turkey",
"underwear","vacation","vegetable","visitor","voyage","year","able","achieve","acoustics","action","activity","aftermath","afternoon","afterthought","apparel","appliance","beginner","believe","bomb","border","boundary","breakfast","cabbage","cable","calculator","calendar","caption","carpenter","cemetery","channel","circle","creator","creature","education","faucet","feather","friction","fruit","fuel","galley","guide","guitar","health","heart","idea","kitten","laborer","language","lawyer","linen","locket","lumber","magic","minister","mitten","money","mountain","music","partner","passenger",
"pickle","picture","plantation","plastic","pleasure","pocket","police","pollution","railway","recess","reward","route","scene","scent","squirrel","stranger","suit","sweater","temper","territory","texture","thread","treatment","veil","vein","volcano","wealth","weather","wilderness","wren","wrist","writerable","achieve","acoustics","action","activity","aftermath","afternoon","afterthought","apparel","appliance","beginner","believe","bomb","border","boundary","breakfast","cabbage","cable","calculator","calendar","caption","carpenter","cemetery","channel","circle","creator","creature","education","faucet","feather","friction","fruit","fuel","galley","guide","guitar","health","heart","idea","kitten","laborer","language","lawyer","linen","locket","lumber","magic","minister","mitten","money","mountain","music","partner","passenger","pickle","picture","plantation","plastic","pleasure","pocket","police","pollution","railway","recess","reward","route","scene","scent","squirrel","stranger","suit","sweater","temper","territory","texture","thread","treatment","veil","vein","volcano","wealth","weather","wilderness","wren","wrist","writer",NULL};

static char *FireboardList[] = {"shyguy58 has been literllyy BEEN ON ALL DAY AND HAS NOT POSTED A THING, MAYBE HE IS A SPY AND READING WHAT WE ALL WRTE ABOUT ACMLM'S BOARD! OR HE COULD BE AN INNOUCENT USER. But its suspicois.",
"An Acmlm's oad is a very hard board to hack and trying to hack the lastest one is even harder, I'm not an expert at hacking but I know that Roger can hack that board. She shut down like 5 boards. To me he is a board hacker.",
"Lightingbolt just said we are in troble.whats that spose to mean? SOrry abiut the other pm I exadently hit enter then sent it.",
"The damn bastards hacked the games! Look at the scores! They weren't like that before. Lets BOTH tell firedude.",
"Go to general chat and look at the forum 'we ar being watched'.",
"I can pm Firedude on the board he gave me at atFreeforums.com\tbog sich somtimes he comes to tat site.And Ill PM Firedude on Fireboard. Do you think thats a good ideal",
"That Basterd! Acmlm put some code so Acmlmers can hack our Arcade Games look at the scores!",
"LOL. No. IP changed to re-reg at Acmlm's Board. LOL its says Korea, before it said China.",
"So what are you doing?If your not busy do you want to play some arcade games,im board.",
"I saw this thread in acmlm and i post one here.Post your video game fear here. I was scared of biolizard in sonic adventure 2 battle the first time i saw it I got scared and couldnt sleep that night.",
"Thats ok I think shitland is going to lose its members any way if they keep doing what they are doing,and the members from shitland will or might reg on Fireboard. Then shitlans will be introble becuse basiacly now one will be thare. Then there going to be coming to us for help.",
"What happen?? Fireboard has died???",
"I've been banned here!! OMG! :(",
"DID YOU JUST HACK OUR BOARD? IDIOT!",
"But i'm gonna delete my account... I'll be back...",
"nvm, we now have a new secret location. LOZA! HAHALOZERZ --spiderman3",
"<spiderman3> Congralations on being a moderator!!! <kilodude> lol tnx man",
"How come it looks like everyone thats on fireboard is banned in acmlms,Ifyou don't mind me asking",
"XKEEPER YOUR MAKING ME ANGRY QUIT QUOTING ME OR I WILL NON-STOP FLOOD THIS BOARD! IT NEVER GET'S OLD --SPIDERMAN3",
"PSA: Hey kids! Spiderman3's password is 'testman' -- have fun! :) ~Management",
"<kilodude> Hey firedude, I just made an account in Acmlm's Board, shoud I flame them or wait till everyone is ready or something?",
"<Help> My spiderman3 account is banned. I made this as a temporary account. <Xkeeper> yeah, he changed it, from 'testman' to 'sm3again'... just barely longer but not at all better",
"<firedude>the server's real staff told me to ip ban him >_> <Watcher> Wait before you do that PM him and tell him the truth. Just to rub it in his face that we are not idiots.",
"<firedude> your password longer and harder to guess,because you curently have a 4 letter pass. <Commentary> longer and harder is always better!",
"Yes,they destroyed the entire board,but the back up did the trick.",
"<LK Invisable> Are you mad at me?Did I do anything that I wasn't spose to do? <Watcher> No, you didn't do anything, what makes you think I'm mad? <LK Invisable> It seems like know ones doin any thing. And mostly when that happens Im always introble",
"<LK Invisable> You should go to acmlms and look under general chat then look under why xkeeper suck and why. I helped out Firedude by getting a new username thare.",
"From: LK invisable -- Subject: Ok know im pisseed How do we contact firedude. // Firedude is not online right know and on simon said I got 33 and on dk i got 34000.How do we tell him",
"<Watcher>Can I say to people something like \"If you join the more chances you have to be admin\"? The majority reason I joined was cause I was banned. But most people are not banned. So can I ask them the question? <firedude> To that i say no,I dont want my board to get full of people wanting to be admins,is best to pm people that have been less than 30 days on the board.Everytime there is a new user pm him.And look in the other sites i pm'd not just acmlm.Gamefaqs has more than 6000 users.you could go there",
"Well tell him there is an anti flaming rule here so he can come here. flaming=ban so that protects him from getting flamed plus im making a profanity filter wich changes bad word like 'you suck', into 'you ****'",
"I didn't knowthe scores were hacked! Why did you do that! You ruined peoples hard work and determination, people played hours on those games to get those scores!",
"Its me Drasen. So uh why don't you introduce yourself in the Newbie Introduction so people can welcome you and stuff. You should be smasrt enough to find it.",
"Well, from what I read, it was like 'Whenever I know the secret link, I'm going to give it out so you can destroy the secret board'. I read it between lines, but I understood that when the boards is his, he will give away the link to acml mers.",
"<LK Invisable> Is Game midi Files allowed here? They are no more then 5MB.Alot less then Mp3,WMa,avi,etc <firedude> yeah,midis arent that heavy but comprense them before uploading it saves more time.",
"Actually TNF went back in time so nows its fine. System Restore I'm guessing.",
"guys we should take them on they are azzholes and our board is beter than there's anytime p.s. can u guys hack into it and get to be administratrs to unban me",
"Almls board sucks.They disabled my PM and know the have it whare everytime I login dhares different colores moving, and saying im doomed Go ahead and try it",
"But then unless we get new members, everyone will be a mod.Commentary: probably the only logic ever used on this board",
"When I type a b, it spits out ASS. It's quiet odd.",
"What does ASS have to do with retard?",
"Ip banning is different if you ban someones ip he wont be able to enter the board untill he disconnects from the internet and connects again,unless I ban all his ips wich could permanently restrict his acces to the board.But is dangerous the ip banning since some users have similar ips I could accidentally ban an inocent user.",
"Forgot ASSout him.",
"my life cereal is too soggy to eat. This ruins my whole morning.",
"I never have been good at timing... ( Remember ask the girl out BEFORE asking her to marry you !!! )",
"The beanstar was a star full of energy but one day the star energy got divided by 7 and there where 7 different stars,creating a chaos in the bean bean kingdom,Mario and his allies wich I wont spoil must collect all seven stars from different lands and bring them to the beanbean castle to turn them into one once again.Ofcourse lots of troubles happen during the way to the end.",
"I mean, I did spend a long time on it... sniff sniff",
"<Jovert> How do i find offsets? I mean like ghettoyouth he posted lots of offsets for people to change. <knog> I think you need to use a tracer or a debuger.",
"Imaturity.",
"Its time to let them know that will never give up.", NULL};

static char *AlabashQuotes[] = {
  "<Alaura> And novayoshi is a fucking prick / <Alaura> Who im going to get banned from this network",
  "<Alaura> Uhhh this is a federal law of the united states / <Alaura> You can not harrass people over any kind of transport system",
  "<Alaura> So i could get this network shut down for it too, by the us goverment / <Alaura> Like they did with megaupload",
  "<Alaura> Or i could always get it shut down by anon / <Lore> Alaura, you don't the first thing about anon, no one is gonna rally your cause / <Alaura> Ya they will / <Alaura> Because i've been helping them shut down websites too / <Alaura> By visiting there sites and there irc",
  "<Alaura> What if i get you banned from this network by the us goverment, hmm? / <Alaura> Because this \"is\" considered cyber bullying / <Alaura> And it is a federal law",
  "<Alaura> Look at esper, if a user gets harrassed, then they get banned / <Alaura> Look at anthrochat, a user gets harrassed, they do nothing but sit on the god damn ass",
  "<Alaura> Fuck my parents, fuck them to fucking god damn hell / <Alaura> I'm fucking pissed at them / <Alaura> Because they buy my fucking little sister a iphone / <Alaura> And they get me a fucking blackberry / <Alaura> God fuck them",
  "<Alaura> Lore fuck you, if you didn't see what he said on my fucking phone / <Alaura> Im sorry lore, just, im sorry, its quite hard to control my temperment around enzo when he used his phone to send me a message .-.",
  "<Alaura> Mood swings eh / <Alaura> There not mood swings /<Alaura> Your just a fucking moron",
  "<Alaura> You knew if i didn't even stay with you, you were a fucking ass and wanted to be killed / <Alaura> That is why, if i ever have to be force to stay with you, i will kill myself in real life instead of fucking staying with you",
  "* Alaura goes to the internet to find information about sucidal acts, but just study's it for other uses",
  "<Alaura> Because people have the freedom of speech :P / <Alaura> Atleast my mistress belives in that right .-.",
  "<Alaura> I still don't get how come NovaYoshi thinks im immature at having op",
  "<Alaura> And, one more thing., when nova gets back, he can and perhaps will kick and ban you",
  "<Alaura> You losing me because your fucking pet is too much of a fucking asshole who needs to get a real attitude and behavior adjustment",
   "<Alaura> Because your afraid to kick someone / <Alaura> Automatticly that makes you a wuss",
// 17
   "<NovaYoshi> Alaura are you being emo-ey / <Alaura> no / <Alaura> Just being intensely suicidal, and i love to hurt myself when i feel this way.",
   "<NovaYoshi> Mine, forever! <3 / <Alaura> Yep / <Alaura> And if i threaten otherwise its fake <3",
   "* Alaura looks at all the drama she creates and all the shit about me being a victium or however she spells it, and looks how she is more annoying to more people than helpful to others, and see's why people don't like her talking at all",
   "<Alaura> Vince will leave me too / <Alaura> If you do i swear / <Alaura> I really something swear / <Alaura> Please don't / <Alaura> Please don't do it. / <Alaura> No, god damn no please, please please please / <Alaura> Fuck please",
   "* Alaura wonders what gives mistrsss the idea that she can massage her breat's",
   "<Alaura> Mistress / <Alaura> Spam on roblox is fun :3",
   "<Alaura> I know mistress's personality, and etc / <Alaura> And i know he isnt mature <3 / <Alaura> but / <Alaura> I still love him all the time",
   "* AlauraPet hides in her corner to avoid the drama",
// 25
   "* Alaura is away: Big Emergency, may burn the house down, and possibly kill her / <Alaura> Nova, Windows hates me... But in other words i just fixed my computer from catching my house on fire / <NovaYoshi> why would it catch your house on fire / <Alaura> Uhh hard drive malfuction / <Alaura> Spark + Wood = Fire",
   "<Alaura> If you do not shut the hell up Ezno i will fucking kill your mother fucking ass",
   "-MemoServ- Haha very funny, i bet vanilla won't like that, and i bet that thats actually cyber bullying, and actually a online threat. so i can and will report you, you know that right? if you don't too fucking bad.Of women",
   "* Alaura plants tnt at the school, and puts a remote detonation in mistress's hand",
   "<Alaura> Also skiy No fucking way will i get my webcam hooked up / <Alaura> No fucking way",
   "<@Aura> Fuck off you homophobic hitler",
   "is suicide worth anything, i have no idea. yet seemingly as most people hate me on irc. it seems promising.",
   "Auras Number (918) 630-0170 Hostname that i love: i.love.to.lick.auras.chocolatey.pussy.us.to || E Out of DATA, 0:1 || Love sexual things :o"
};

static char *SpambashQuotes[] = {
  "Yea, Still Bankhead u can tell, aint nothin change but the name on second hand cars the mail! aq jg fan nya barca,ris nervous I'm here with Diego (: such a great dog!!",
  "When you're feeling a bit ropey nothing quite hits the spot like these PreMatchMeal tienes novio? si// no me gusta nada. yea so were bestfriends haha :p",
  "<3 Encore Oil's drilling at Cladhan field comes up 'water wet'. Shares gone south fast. Investing in oil and gas exploration one heck of a risk",
  "genios tus piojos Here's a printscreen from my phone!!! :) OMB SUSKSHANISSBHSJSV The Catchup Lounge Tomorrow At 9PM Hosted by It will never be approved so I won't have to worry. :)",
  "I no so wen we gone do this cuz I need to ASAP Best man is the world mwahhhhhh tony Ah, and there was I thinking you were being Elusive.",
  "There's your air of mystery blown.... Toronto : oh, I've missed this video. Adam Lambert chats with Erin & Mike",
  "[Bob Marley] i heard 3 years ago today i did my first message. wild. alot has changed. thanks for growing with me and taking the journey. more to do. My cat puked today. Followed :)",
  "Cuarenta naipes han desplazado a la vida. ElTruco Happy Birthday to my best friend ... We live Queens Brilliant! \"KINO RETURNS. The short film event BY filmmakers FOR filmmakers. Nothing programmed. Everything possible!\"",
  "ps: yellow nails? Did you? X Hi Mike, it's an excellent read, well worth buying. Hope you are well, will be in touch when back in the UK.",
  "This is me muting my stream domestic abuse and making a phone call LA VENGANZA SE SIRVE FRÍA El número",
  "love Is Up We have finally discovered a use for query_posts(): quickly identifying red flags. such a good night, cut my finger on a can lid though, woop!",
  "now following! Sexual Wednesday.... Let's party. InYoPants I intentionally bury funny gifs in the deluge of tabs I cultivate each day so I can rediscover them like easter eggs as I work",
  "\"It doesn't matter who you are, where you came from. The ability to triumph begins with you. Always.\" Quotes Laissons faire la justice FH2012",
  "Im bored Im bored Im bored Im bored Im bored Im bored Im bored Im bored Im bored Im bored Im bored IM pr agency BORED",
  "Rt for a shoutout i'll try and do as many as I can. They did last year. nf followback HBCUTRAIN is the only H word that Jamaicans pronounce...",
  "And with emphasis on the H buen dia Pela! Podes RT ? Hoy en lacocinadelshow Bad belle sumbody\": Time out...\": Refresh...\"",
  "not really, I slept on my arm and now it's numb. PLEASEEEE CLICKKKKK THIS Moms just called me and said CD is sold out...Bouta have a HeartAttack!!! IWaitedToLong!",
  "UEFA charging england fans on an apparent pitch invasion Also please feel free to leave comments and ask questions at the bottom of the blog.. HAPPY SUNDAY! Boston tomorrow! CANwnt",
  "tacomovies HAR Y POTTER ODRER OF TACO YESSS YESSSS *COMEDY CROWN IS PLACED ON HEAD* Paranormal Activity 3 trailer, never knew they was bringing another one out",
  "Niall you beast Freezing my nose off in a practice room, whoever was using it last had the thermostat pr agency on 50!",
  "For the first time in four games, the will play an opponent that is playing for something. trying to sneak into the playoffs. Thank you! We try :)",
  "My favorite rapper of all time is Biggie Smalls. Even doe he aint have that many songs.",
  "yes Nove horas já -'- Only 16% ease coffee machines of recent high school grads have full time jobs. That's why I wrote this: haha tunes thanks for listening :D",
  "\"know which great American poet taught me that? Henry Ford. He wrote poems so crazy they came out as cars.",
  "Derek Fisher taking \"if you can't beat them, join them\" a little TOO literally for my liking... GN luv....I miss you SALTYDOG 2nd. sample-09. book ebook authors cool pixiv When jealous girls try to start drama -_- haters Hidden valley later .",
  "haha lol, we're just lonely.. But i really have fun with my posters. \"People are coming down the red carpet\" OMFGGGGGGGGGGGGGGGGGGGGGG EMMA PLEASE SAY EMMA PLEASE EMMA",
  "especially the big accounts. Except I love her. \"He never looked so presidential, never sounded so sincere.\" watches Romney react to scotus decision",
  "Thank you! :) x you don't seem like an exciting person, I don't like dull people e debt management iyi o zaman I can't sorry I'm on my phone, try just typing it into YouTube xx :)",
  "OURLOVEpreview trend it for :) obviously the 1st 3 were jokes but the 2nd 3 I'd like, not sure what he'd do with Johnson, maybe right side? What dya reckon? we our youngggg!!!!",
  "Hay maneras, hay formas.\" // porsupuesto, para todo. New Avi I see you NEW BLOG POST: Pink Barbie doll shoes",
  "lmao im dying to know xx X_X.** Start a dating service! \": 2,000 people showed up on Ustream, on a Friday night, to watch 's Live USMNT coverage\" Niall's nude picture :",
  "WhatCollegeHasTaughtMe To Appreciate Every Nap second hand cars I Can Get",
  "Thanks Mr Hawkins Give Mr Wilson a pat on the back and an atta boy for debt management me PMAngels‎;",
  "Part 3 of our investigative series from deep in the rubber room at domestic abuse Lincoln Center storyhack?",
};

static char *DerpbashQuotes[] = {
"<TFG_Fursona> Of course I'm sexy / <TFG_Fursona> I have a giant cat dong",
"<Mew> The country of the day is nigger / <Mew> niger*",
"<Mew> Windows2008R2 Uptime: 6days 6hrs 1min 8secs Best: 6days 20hrs 21mins 6secs / <Mew> hm / <Mew> watch it crash before it hits that mark / <NovaSquirrel> it will crash into 50000 dave sized bits / <Mew> windows tends to need a lot more space than linux / <Mew> NovaSquirrel I will beat your ass",
"<Mew> ok so i wouldbhave had an opportunity to get myself beaten up. a high schooler was all \"he can suck my fucking dick \" and I almost said \"that is the gayest thing I've ever heard\" / <Mew> this is an antigay community / <Mew> so that would have been a very high insult",
"<TFG_Fursona> ._. / <TFG_Fursona> I still want sexytimes with my Mewmew / <Mew> -_- / <Mew> mewmew just did his Eng paper	 / <TFG_Fursona> MEANWHILE / <TFG_Fursona> I'M BORED",
"<TFG_Umbreon> So Phaze / <TFG_Umbreon> My defense is non existant / <TFG_Umbreon> Nor is my health / <TFG_Umbreon> Y U NO LEVEL ME UP",
"* TFG_Umbreon Doesn't want to Rape mew but thinks about it",
"<Umbreon> I'm trying my best to not overreact, NovaSquirrel. / <Umbreon> and failing to overreact would represent one who can handle an O:Line",
"<Saturn> so how come i cant update the topic? / <Mew> Saturn because people change it to things it shouldn't be changed to",
"* TFG_Lucario Looks up under NovaSquirrel's dress / <NovaSquirrel> yeah have fun looking at my panties / <TFG_Lucario> You do realize I can see through clothing right / <NovaSquirrel> what's the point of looking up my dress then",
"<NovaSquirrel> * Failed OPER attempt by Mew (Mew@mews.ip) [unknown oper] / <Mew> yes / <Mew> that's cranked's assfault / <NovaSquirrel> rofl did you really believe he would let you keep your oblock / <Mew> I thought he would forget about it",
"<NovaSquirrel> is saturn a Sega Saturn and cinos a Sega Genesis / <Mew> no / <NovaSquirrel> why / <Mew> Saturn is 100x the sega saturn and cinos isn't even close to genesis",
"<Mew> I want / <Mew> my grandpa / <Mew> to give me / <Mew> my cat back / <Mew> and die",
"* TFG_Fursona Humps NovaSquirrel's leg / * TFG_Fursona Explodes on Nova's leg",
"<Nightfury> no / <Nightfury> Xandra I was horny as fuck earlier and I fixed that with a straw / <Xandra> How / <Nightfury> that's personal",
"<Umbreon> Dingo is mine / <TFG_Fursona> I love Dingo / <Umbreon> He's mine / <TFG_Fursona> He's mine too / <Umbreon> not your lover / <TFG_Fursona> I had sex with him last night / * Umbreon glares / * TFG_Fursona Shrugs",
"<Mew> I'm insulted by cranked slightly but it's nothing he's actually said this time",
"<TFG_Fursona> IOTA / <TFG_Fursona> LICK ME",
"<Mew> I'm taking a poop!",
"<TFG_Fursona> I shall now have a wet dream about Mew",
"<TFG_Fursona> Mew's penis is so giant / <TFG_Fursona> He can kill anyone with it",
"<TFG_Fursona> I don't like that he's a pacifist or however you spell it. / <TFG_Fursona> How can you stand and do nothing to contribute to life whatsoever?	 / <Saturn> that's not what a pacifist is",
"<TFG_Fursona> And I dislike pacifists just because religion / <TFG_Fursona> I dislike religion. / <Saturn> religious people are so RARELY pacifist",
"<TFG_Fursona> Cat dong time / * TFG_Fursona Waves his dong in everyone's faces",
"* Umbreon rapes TFG_Mew. / * TFG_Mew Loves it / * TFG_Mew Rapes Umbreon / * Umbreon loves it. / * TFG_Mew Wants details with Umbreon soon",
"* Sattykins cut off TFG_Mew's penis / <TFG_Mew> Well / <TFG_Mew> I had to pee / <TFG_Mew> OWWWWWWWWWWWWWWWWWWWWWW / * TFG_Mew Regrows his penis",
"* TFG_Mew Urinates on a plant / <Umbreon> TFG_Mew you're only allowed to do that when you're Scourge or in Fursona form",
"* TFG_Mew Licklicks Umbreon's special area / <TFG_Mew> Your knee tastes funny / <Umbreon> TFG_Mew yes I know it does but you're licking my dong. / <TFG_Mew> And that is nummy",
"<TFG_Umbreon> Kephyr is super awesome and super sexy and super sized and super nummy and super super and super super super-r-r-r-r-r-r-r-r-r-r-r / * TFG_Umbreon .exe has stopped responding",
"<TFG_Umbreon> Sattykins / <TFG_Umbreon> I need to be suckled / <TFG_Umbreon> HOW DO",
"* Flora-X Fingers herself on the couch / <Flora-X> FINGERS / <Flora-X> OF CHEESE",
"<Aeris> I'M A CAT / <Aeris> CAT BOOBS",
"<TFG_Fursona> Trying to prop Umbreon up so I can look at his pokeballs. / <Umbreon> what do you hav efor my balls / * TFG_Fursona Looks at Umbreon's special are / <TFG_Fursona> I just want to stare at them / <Umbreon> stop staring at my testicles / <TFG_Fursona> Why / <TFG_Fursona> They're big / <Umbreon> I don't want you looking at my genitals",
"<TFG_Umbreon> I'M AN UMBREON / <TFG_Umbreon> FEAR MY... SOMETHING... NovaSquirrel",
"* TFG_Fursona Gets an erection and stares at everyone / <TFG_Fursona> WHAT DO I DO",
"<Mew> so anyway I found out why when I walk somewhere I smell shit / <Mew> there's shit on the bottom of my shoes / <Mew> I've shitting stepped in shit and it smells"
};

static char *DrunkbashQuotes[] = {
"<RaivenDidelphis> =w= im fdrink",
"* Lusan tisp ASH over / <Lusan> ffffloppy foxieeeeeeeee",
"* Lusan shdnuuuuuuuuuuuugles on INzzy",
"<Lusan> Stredi! / <Lusan> fusk / <Strife> hehe / <Lusan> STridef! / <Strife> have you been drinking, Lusan? / <Lusan> onoly teeeensy bit",
"<Lusan> thatsss fuckiijn nasthyh / <Lusan> nastyr / <Lusan>  eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeewwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww",
"* Lusan hugsus on oooooooooooo fdub",
"<Lusan> im finie / <Lusan> fine / <Lusan> Im fien / <Lusan> FUCK",
"<Lusan> amnoat drunk / <Lusan> jstu t33333333qqilqa",
"<Lusan> fucikingg / <Lusan> an / <Lusan> ananananananan / <Lusan> bannaana",
"<Lusan> sillyyy / <Lusan> xxxxxxxxxx3444443 / <Lusan> x3",
"<Lusan> shanesm / <Lusan> em / <Lusan> mean / <Lusan> menae beastard / <Lusan> mean bastarad",
"* Lusan kiesisssssssssssssssssssssssssssssssssssss fxo / <Lusan> fox / <Lusan> .me fosx kkissssss'",
"<Lusan> shane is a cujntmuffin / <Lusan> cuntmuffin / <Lusan> muffincunt / <Lusan> SHANE HAS S  MIIFFFFFFFNCUNT",
"<Lusan> Shnae / <Lusan> shane / <Lusan> meanbasatard / <Lusan> sowaht fi i hae a cunnt",
"<Lusan> ffffffffffffffffffffffffffffffffffffffffffff / <Lusan> fcik / <Lusan> FUKC / <Lusan> FUCK / <Lusan> you pnenishead",
"<Lusan> totoooooooooooooooooooooooooooooooooooooooooooooooooooooph",
"<Lusan> sahneeee / <Lusan> Shane / <Lusan> hes a mena bastarsda / <NovaSquirrel> and you're drunk / <Lusan> hes a maen bastrad / <Lusan> amnot",
"<Lusan> hes a musssincutnn toooooooo / <Lusan> mduffincnt / <Lusan> muffin / <Lusan> muffincunt / <Lusan> cuntnmuffi",
"<Lusan> Shane / <Lusan> Shane / <Lusan> Canwe ahve makeupsex / <Lusan> b/c / <Lusan> that makes everyhting better",
"<Lusan> toph / <Lusan> watch uot / <Lusan> you might get hte / <Lusan> \the / <Lusan> muffincunt / <Lusan> from oShane / <Lusan> Shane",
NULL
};

static int VerbCount = 0;
static int NounCount = 0;
static int FireboardCount = 0;

static int strchr2(const char *FindIn, char FindMe) {
  char *a = strchr(FindIn,FindMe);
  if(a == NULL) return -1;
  return a-FindIn;
}

static char *ConvertString(char *Output, const char *Input, const char *Control) {
  if(strlen(Control) < 2) return "Control string too short";
  strcpy(Output, "");
  char Temp[32], *Poke = Output, SeparateWith = ' ';
  const char *Peek;
  int SrcFormat=-1, DstFormat=-1;
  SrcFormat = strchr2("xdc", Control[0]); if(SrcFormat == -1) return "Invalid source format";
  DstFormat = strchr2("xdc", Control[1]); if(DstFormat == -1) return "Invalid destination format";
  if(strchr(Control,',')) SeparateWith = ',';
  if(SrcFormat == 0 && !(strlen(Input) == strspn(Input,",0123456789abcdefABCDEF ")))
    return "Your input string is weird :(";
  if(SrcFormat == 1 && !(strlen(Input) == strspn(Input,",0123456789 ")))
    return "Your input string is weird :(";
  for(Peek=Input;*Peek;) {
    char k;
    switch(SrcFormat) {
      case 0:
        k = strtol(Peek, NULL, 16);
        while(isxdigit(*Peek)) Peek++;
        if(*Peek==',') Peek++;
        break;
      case 1:
        k = strtol(Peek, NULL, 10);
        while(isdigit(*Peek)) Peek++;
        if(*Peek==',') Peek++;
        break;
      case 2:
        k = *(Peek++);
        break;
      default:
        return "\2:(";
        break;
    }
    if(k=='\n' || k=='\r') return "\2:(";
    switch(DstFormat) {
      case 0:
        sprintf(Temp, "%.2x", k&255);
        strcpy(Poke, Temp);
        Poke += strlen(Temp);
        break;
      case 1:
        sprintf(Temp, "%.2d", k&255);
        strcpy(Poke, Temp);
        Poke += strlen(Temp);
        break;
      case 2:
        *(Poke++) = k;
        break;
      default:
        return "\2:(";
        break;
    }
    if(SrcFormat != 2 && strpbrk(Peek, "0123456789abcdefABCDEF") == NULL)
      break;
    if(DstFormat != 2)
      *(Poke++) = SeparateWith;
  }
  *Poke = 0;
  if(strlen(Output)<420) return Output;
  return "\2:(";
}

/*static char *Backwords(const char *Inp, char *Out) {
  int i, temp, Len; char *Seek = Out, *Fix;

  for(i=strlen(Inp)-1;i!=-1;i--)
    *(Seek++) = Inp[i];
  *Seek = 0;

  for(Seek = Out;;Seek++) {
    if((Seek!=Out) && ((*Seek == ' '||!*Seek) && Seek[-1]!=' ')) {
      for(Fix = Seek;Fix != Out && Fix[-1] != ' ';Fix--);
      if(NULL!=strchr(Fix, ' ')) Len = strchr(Fix, ' ') - Fix;
      else Len = strlen(Fix);
      for(i=0;i<=(Len>>1)-1;i++) {
         temp = Fix[i];
         Fix[i] = Fix[Len-1-i];
         Fix[Len-1-i] = temp;
      }
    }
    if(!*Seek)
      break;
  }
  return Out;
}*/

static char *RandChoice(const char *ChoiceStr, char *Buffer) {
  char *Choices[32];
  int NumChoices = 0;
  strcpy(Buffer, ChoiceStr);
  char *Seek = Buffer;
  while(1) {
    Choices[NumChoices++] = Seek;
    if(NumChoices >= 32) return "Too many choices";
    Seek = strchr(Seek, '/');
    if(Seek == NULL) break;
    *(Seek++) = 0;
  }
  return Choices[rand2(NumChoices)];
}

static int ThisYear() {
  time_t RawTime;
  struct tm *TimeInfo;
  time(&RawTime);
  TimeInfo = localtime(&RawTime);
  return TimeInfo->tm_year;
}

/*static int autoupdate_logs_cb(void *userdata) {
  system("/home/nova/bin/updatelogs");
  return 1;
}*/

static int ReadDate3(const char *Peek) {
  if(!strcasecmp(Peek, "now"))
    return time(NULL);
  int Month, Day, Year;

  if(!Peek[0]) return -1;
  Month = strtol(Peek, NULL, 10);
  while(*Peek == ' ') Peek++;
  if(*Peek=='/') Peek++;
  if(!Peek[0]) return -1;
  Day = strtol(Peek, NULL, 10);
  while(*Peek == ' ') Peek++;
  if(*Peek=='/') Peek++;
  if(!Peek[0]) Year = ThisYear();
  else Year = strtol(Peek, NULL, 10);
  if(Year < 100) {
    if(Year > 70)
     Year+=1900;
    else
     Year+=2000;    
  }

  time_t RawTime;
  struct tm *TimeInfo;
  time(&RawTime);
  TimeInfo = localtime (&RawTime);
  TimeInfo->tm_year = Year - 1900;
  TimeInfo->tm_mon = Month - 1;
  TimeInfo->tm_mday = Day;
  return mktime(TimeInfo);
}

static double Square(double V) {
  return V*V;
}
static double Pythag(double Hyp, double Leg1, double Leg2) {
  double Temp;
  if(Hyp == 0 && Leg1 > 0 && Leg2 > 0) // find hypotenuse
    return sqrt(Square(Leg1)+Square(Leg2));
  if(Hyp > 0 && Leg1 == 0 && Leg2 > 0) {
    Temp = Leg2;
    Leg2 = Leg1;
    Leg1 = Temp;
  }
  if(Hyp > 0 && Leg1 > 0 && Leg2 == 0)
    return sqrt(Square(Hyp)-Square(Leg1));
  if(Hyp > 0 && Leg1 > 0 && Leg2 > 0) {
    if(Square(Hyp)==(Square(Leg1)+Square(Leg2)))
      return -2;
    else
      return -3;
  }
  return -1;
}

char *VDPAddrSwap(const char *Input, char *Output) {
  while(*Input && (*Input=='$'||*Input=='#')) Input++;
  unsigned int Temp;
  if(strlen(Input) == 4) {
    Temp = strtol(Input, NULL, 16);
    sprintf(Output, "%.4x000%x", (Temp & 0x3fff) | 0x4000, (Temp&0xc000) >> 14);
    return Output;
  } else if(strlen(Input) == 8) {
    char Buf[16];
    strcpy(Buf, Input);
    Buf[4] = 0;
    Temp = strtol(Buf, NULL, 16) & 0x3fff;
    Temp+= strtol(Buf+6, NULL, 16)<<14;
    sprintf(Output, "%.4x", Temp);
    return Output;
  } else {
    return "Address must be either 4 or 8 characters long";
  }
}

static char *BruteforceKrypto(int n1, int n2, int n3, int n4, int n5, int Desired, char *Poke) {
  int N[5] = {n1,n2,n3,n4,n5};
  int A,B,C,D,E,i;
  int Op[5];
  const char OpChar[4] = {'+', '-', '*', '/'};

  for(A=0;A<5;A++) {
    for(B=0;B<5;B++) {
      if(B==A)
        continue;
      for(C=0;C<5;C++) {
        if(C==A || C==B)
          continue;
        for(D=0;D<5;D++) {
          if(D==A || D==B || D==C)
            continue;
          for(E=0;E<5;E++) {
            if(E==A || E==B || E==C || E==D)
              continue;
            // will loop 120 times

            float W[5] = {N[A], N[B], N[C], N[D], N[E]};
            float Val=W[0];
            for(Op[0]=0;Op[0]<4;Op[0]++)
              for(Op[1]=0;Op[1]<4;Op[1]++)
                for(Op[2]=0;Op[2]<4;Op[2]++)
                  for(Op[3]=0;Op[3]<4;Op[3]++) {
                    for(Val=W[0], i=0; i<4; i++)
                      switch(Op[i]) {
                        case 0:
                          Val += W[i+1];
                          break;
                        case 1:
                          Val -= W[i+1];
                          break;
                        case 2:
                          Val *= W[i+1];
                          break;
                        case 3:
                          Val /= W[i+1];
                          break;
                      }
                    if(Val == Desired) {
                      sprintf(Poke, "%2i %c %2i %c %2i %c %2i %c %2i = %2i \n", 
                     (int)W[0], OpChar[Op[0]], (int)W[1], OpChar[Op[1]], (int)W[2], OpChar[Op[2]],
                     (int)W[3], OpChar[Op[3]], (int)W[4], Desired);
                      return Poke;
                    }
                  }
          }
        }
      }
    }
  }
  return "No solutions?";
}


/*//Convert a string to an integer, and notes a '%' or '$' starting a number
static int ConvertCA65Num(char *In, char **Out) {
  char *Peek=In;                //Start at the input string
  while(isspace(*Peek)) {       //Skip to the end of any leading whitespace before the number
    Peek++;                                        
    if(*Peek=='\0')                    //Handle the whole string being whitespace, if it happens
      return(0);
  }
  if(*Peek=='$')                       //Prefixed with a '$' = Hexadecimal
    return(strtol(Peek+1, Out, 16 ));
  if(*Peek=='%')                       //Prefixed with a '%' = Binary
    return(strtol(Peek+1, Out, 2 ));
  return(strtol(Peek, Out, 10 ));      //Not prefixed with anything = Decimal
}*/

static char *z80FakeMultiply(char *Result, int Find) { // nb.zmulbyadd
  char Temp[100];
  Temp[99]=0;     // String end
  if(Find==-1){strcpy(Result,"neg");    return(Result);}
  if(Find==0){strcpy(Result,"xor a");   return(Result);}
  if(Find==1){strcpy(Result,"nop");     return(Result);}
  if(Find<0){strcpy(Result,"Only positive numbers, please");  return(Result);}

  char *Poke = Temp+98;
  while(Find != 1) {
    if(Find & 1) {
      Find--;
      *(Poke--) = 'y';
    } else {
      Find>>=1;
      *(Poke--) = 'x';
    }
    printf("%i ", Find);
  }
  strcpy(Result, ++Poke);
  printf("\nResult is %s \n", Poke);
  return(Result);
}

static void z80Asm(const char *Input, char *Output) {
  remove("/home/nova/junk/zas.bin");
  // First we need to create a file to feed z80asm
  FILE *MyFile = fopen("/home/nova/junk/zas.asm","w");
  if(MyFile==NULL) {
    strcpy(Output,"unable to open zas.asm");
    return;
  }

  // Intercept all '\'s and turn them into newlines
  const char *Peek = Input;
  while(*Peek) {
    char got = *Peek;
    if(got == '\\') { // new line
      fputc('\r',MyFile);
      fputc('\n',MyFile);
    }
    else
      fputc(got, MyFile);
    Peek++;
  }

  // Now we have a file to feed z80asm
  fclose(MyFile);

  char Command[512];
  sprintf(Command, "z80asm -i /home/nova/junk/zas.asm -o /home/nova/junk/zas.bin");
  int Status = system(Command);

  // Open the assembled code
  MyFile = fopen("/home/nova/junk/zas.bin","rb");
  if(MyFile==NULL) {
    sprintf(Output,"Unable to open output file? (system() returned %i) I'm guessing you made a mistake in your code", Status);
    return;
  }
  
  // Now get the hex values from the compiled code
  strcpy(Output,"");
  char *Poke = Output;
  // int len=0; // code length
  int ch;
  do {
    char Small[7];
    ch = fgetc(MyFile);
    if(ch==EOF)
      break;
    sprintf(Small, "%.2x ",ch);

    if(strlen(Output) > 400) {
      sprintf(Output,"Assembled file too big");
      return;
    }

    strcat(Poke,Small);
  } while(ch != EOF);
}


static void NESAsm(const char *Input, char *Output) {
  remove("/home/nova/junk/zas.bin");
  remove("/home/nova/junk/zas.o");
  // First we need to create a file to feed ca65
  FILE *MyFile = fopen("/home/nova/junk/zas.asm","w");
  if(MyFile==NULL) {
    strcpy(Output,"unable to open zas.asm");
    return;
  }
  fprintf(MyFile, ".include \"nes.h\"\r\n");

  // Intercept all '\'s and turn them into newlines
  const char *Peek = Input;
  while(*Peek) {
    char got = *Peek;
    if(got == '\\') { // new line
      fputc('\r',MyFile);
      fputc('\n',MyFile);
    }
    else
      fputc(got, MyFile);
    Peek++;
  }

  // Now we have a file to feed ca65
  fclose(MyFile);

  // Open the assembled code
  if(system("/home/nova/bin/ca65 /home/nova/junk/zas.asm -o /home/nova/junk/zas.o")) {
    strcpy(Output,"Unable to open object file?");
    return;
  }

  if(system("/home/nova/bin/ld65 -o /home/nova/junk/zas.bin -C /home/nova/junk/6502.x /home/nova/junk/zas.o")) {
    strcpy(Output,"Unable to open assembled file?");
    return;
  }
  MyFile = fopen("/home/nova/junk/zas.bin","rb");
  
  // Now get the hex values from the compiled code
  strcpy(Output,"");
  char *Poke = Output;
  int ch;
  do {
    char Small[7];
    ch = fgetc(MyFile);
    if(ch==EOF)
      break;
    sprintf(Small, "%.2x ",ch);
    if(strlen(Output) > 400) {
       sprintf(Output,"Assembled file too big");
      return;
    }
    strcat(Poke,Small);
  } while(ch != EOF);
}

static char *File2Buffer(const char *File, char *Output) {
  int ReadChars = 0;
  FILE *MyFile = fopen(File,"rb");
  if(MyFile==NULL) {
    sprintf(Output,"Unable to open the file");
    return Output;
  }

  strcpy(Output,"");
  char *Poke = Output;
  int ch;
  do {
    ReadChars++;
    if(ReadChars>500) {
      *Poke = 0;
      fclose(MyFile);      
      return "Too long!";
    }
    ch = fgetc(MyFile);
    if(ch==EOF)
      break;
    if(ch=='\n' || ch=='\r')
      ch = '|';
    *(Poke++) = ch;
  } while(ch != EOF);
  fclose(MyFile);
  *Poke = 0;
  return Output;
}

static char *AVerb() {
  return VerbList[rand2(VerbCount)];
}
static char *ANoun() {
  return NounList[rand2(NounCount)];
}

char *NESGenie(const char *Input, char *Output) {
  char Key[] = "APZLGITYEOXUKSVN", Upper[512];
  int i, Addr=0, Data=0, Compare=0;
  strcpy(Upper, Input);
  for(i=0;Upper[i];i++)
    Upper[i] = toupper(Upper[i]);

  if(strlen(Input) == 6 || strlen(Input) == 8) {
    char n[strlen(Input)];
    for(i=0;Input[i];i++) {
      char *Find = strchr(Key, Upper[i]);
      if(!Find) return "Unrecognized character in code";
      n[i] = Find - Key;
    }
    Addr = 0x8000 + (((n[3] & 7) << 12) | ((n[5] & 7) << 8) | ((n[4] & 8) << 8) \
        | ((n[2] & 7) << 4) | ((n[1] & 8) << 4) | (n[4] & 7) | (n[3] & 8));
    if(strlen(Input) == 6) {
      Data = ((n[1] & 7) << 4) | ((n[0] & 8) << 4) | (n[0] & 7) | (n[5] & 8);
      sprintf(Output, "%.4x %.2x", Addr, Data);
      return Output;
     } else {
      Data = ((n[1] & 7) << 4) | ((n[0] & 8) << 4) | (n[0] & 7) | (n[7] & 8);
      Compare = ((n[7] & 7) << 4) | ((n[6] & 8) << 4) | (n[6] & 7) | (n[5] & 8);
      sprintf(Output, "%.4x %.2x %.2x", Addr, Data, Compare);
      return Output;
    }
  } else if(strlen(Input) == 7 || strlen(Input) == 10) {
    int UseCompare = (strlen(Input)==10);
    Addr = strtol(Input+0, NULL, 16);
    Data = strtol(Input+5, NULL, 16);
    if(!UseCompare) {
      Output[0] = Key[(Data>>4 & 8) | (Data & 7)];
      Output[1] = Key[(Addr>>4 & 8) | (Data>>4 & 7)];
      Output[2] = Key[8 | (Addr>>4 & 7)];
      Output[3] = Key[(Addr & 8) | (Addr>>12 & 7)];
      Output[4] = Key[(Addr>>8 & 8) | (Addr & 7)];
      Output[5] = Key[(Data & 8) | (Addr>>8 & 7)];
      Output[6] = 0;
      return Output;
    } else {
      Compare = strtol(Input+8, NULL, 16);
      Output[0] = Key[(Data>>4 & 8) | (Data & 7)];
      Output[1] = Key[(Addr>>4 & 8) | (Data>>4 & 7)];
      Output[2] = Key[8 | (Addr>>4 & 7)];
      Output[3] = Key[(Addr & 8) | (Addr>>12 & 7)];
      Output[4] = Key[(Addr>>8 & 8) | (Addr & 7)];
      Output[5] = Key[(Compare & 8) | (Addr>>8 & 7)];
      Output[6] = Key[(Compare>>4 & 8) | (Compare & 7)];
      Output[7] = Key[(Data & 8) | (Compare>>4 & 7)];
      Output[8] = 0;
      return Output;
    }
  } else {
    return "Invalid syntax?";
  }
}

static void NESCompile(const char *Input, char *Output) {
  remove("/home/nova/junk/zas.txt");
  remove("/home/nova/junk/zas2.txt");
  FILE *MyFile = fopen("/home/nova/junk/zas.c","w");
  if(MyFile==NULL) {
    strcpy(Output,"unable to open zas.c");
    return;
  }
  fprintf(MyFile, "#define POKE(addr,val)     (*(unsigned char*) (addr) = (val))\r\n");
  fprintf(MyFile, "#define POKEW(addr,val)    (*(unsigned*) (addr) = (val))\r\n");
  fprintf(MyFile, "#define PEEK(addr)         (*(unsigned char*) (addr))\r\n");
  fprintf(MyFile, "#define PEEKW(addr)        (*(unsigned*) (addr))\r\n");
  fprintf(MyFile, "unsigned char x; unsigned char y; unsigned char z;");
  fprintf(MyFile, "void main() {%s}\r\n", Input);
  fclose(MyFile);

  if(!system("cc65 /home/nova/junk/zas.c -Oi -Cl -o /home/nova/junk/zas.txt")) {
    if(system("/home/nova/bin/strip65 /home/nova/junk/zas.txt > /home/nova/junk/zas2.txt"))
      File2Buffer("/home/nova/junk/zas2.txt", Output);
    else
      strcpy(Output, "???");
  } else
    strcpy(Output, "Failed to compile?");
}

static void NBFalse(const char *Input, char *Output, int Program) {
  remove("/home/nova/junk/out.txt");
  // First we need to create a file to feed the false compiler
  FILE *MyFile = fopen("/home/nova/junk/sourcecode.f","w");
  if(MyFile==NULL) {
    strcpy(Output,"unable to open source for writing");
    return;
  }
  int ReadChars = 0;

  const char *Peek = Input;
  while(*Peek) {
    fputc(*(Peek++), MyFile);
  }

  // Now we have a file to feed nbfalse
  fclose(MyFile);

  int Status = system("PATH=\"$HOME/bin:$PATH\"");
  if(Program == 0) {
    Status = system("/home/nova/bin/nsfalse /home/nova/junk/sourcecode.f > /home/nova/junk/out.txt");
  } else {
    Status = system("/home/nova/bin/safesq /home/nova/junk/sourcecode.f > /home/nova/junk/out.txt");
  }
  
  // Open the output
  MyFile = fopen("/home/nova/junk/out.txt","rb");
  if(MyFile==NULL) {
    sprintf(Output,"Unable to open output file. (%i)", Status);
    return;
  }
  
  strcpy(Output,"");
  char *Poke = Output;
  // int len=0; // code length
  int ch;
  do {
    ReadChars++;
    if(ReadChars>500) {
      *Poke = 0;
      fclose(MyFile);      
      return;
    }
    ch = fgetc(MyFile);
    if(ch==EOF)
      break;
    if(ch=='\n' || ch=='\r')
      ch = '|';
    *(Poke++) = ch;
  } while(ch != EOF);
  fclose(MyFile);
  *Poke = 0;
}

static char *EscapeString(char *Buffer, const char *Peek) {
  char *Poke = Buffer;
  while(*Peek) {
    if(*Peek == '\"' || *Peek == '\\')
      *(Poke++) = '\\';
    *(Poke++) = *(Peek++);
  }
  *(Poke++) = 0;
  return Buffer;
}

int BotCmd_BotCommand(BotEvent *Event) {
  const char *Nick = Bot_GetEventStr(Event, "Nick", "?");
  const char *NBCmd = Bot_GetEventStr(Event, "CmdName", "?");
  const char *ArgPtr = Bot_GetEventStr(Event, "CmdArg", NULL);
  if(!*ArgPtr) ArgPtr = NULL;
  char WordBuff[2048];
  const char *word[32] = {""};
  const char *word_eol[32] = {""};
  if(ArgPtr) Bot_XChatTokenize(ArgPtr, WordBuff, sizeof(WordBuff), word, word_eol, 32);
  int i = 0; // compatibility
  char Temp[512];
//  printf("Handling command %s\n", NBCmd);

  char SquirrelProgram[2000];
  if(NBCmd[0] == '$') {
    FILE *SqFile;
    const char *SqName = NBCmd+1;
    if(NBCmd[1] == '+' || NBCmd[1] == '^')
      SqName++;
    CreateDataPath2(Temp, "sqcmd/", SqName, ".txt");
    if(NBCmd[1] == '+') { // add new command
      if(ArgPtr != NULL) { // needs an argument
        SqFile = fopen(Temp, "wb");
        if(SqFile == NULL) {
          Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=Command is read-only\t");
          return ER_HANDLED;
        }
        fprintf(SqFile, "%s", ArgPtr);
        fclose(SqFile);
        Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=Command saved\t");
        return ER_HANDLED;
      }
      else
        Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=Can't write a blank program\t");
      return ER_HANDLED;
    } else if(NBCmd[1] == '^') {
      SqFile = fopen(Temp, "rb");
      if(SqFile == NULL) {
        Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=Does that command exist?\t");
        return ER_HANDLED;
      }
      char *Poke = Temp;
      while(!feof(SqFile)) {
        char a = fgetc(SqFile);
        if(a == '\n') continue;
        if(a == '\r') continue;
        if(a == '\t') continue;
        *(Poke++) = a;
      }
      if(Poke[-1] == EOF) Poke--;
      *Poke = 0;
      fclose(SqFile);
      if(strlen(Temp) < 420)
        Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=%s\t", Temp);
      else
        Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=Command source too long\t");
      return ER_HANDLED;
    } else {
      SqFile = fopen(Temp, "rb");
      if(SqFile == NULL) {
        Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=Does that command exist?\t");
        return ER_HANDLED;
      }
      char *Poke = Temp;
      while(!feof(SqFile))
        *(Poke++) = fgetc(SqFile);
      if(Poke[-1] == EOF) Poke--;
      *Poke = 0;
      fclose(SqFile);

      char TempArg[700]="";
      Poke = TempArg;
      if(ArgPtr)
        EscapeString(TempArg, ArgPtr);

      sprintf(SquirrelProgram, "local A = \"%s\", U = \"%s\";\n%s", TempArg, Nick, Temp);
      NBCmd = "squirrel";
      ArgPtr = SquirrelProgram;
    }
  }

  if((!strcasecmp(NBCmd,"squirrel")||!strcasecmp(NBCmd,"sq")) && NULL!=ArgPtr) {
    if(strlen(ArgPtr) > 600)
      Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=Too many chars given\t");
    else {
      char Hex[2048];
      NBFalse(ArgPtr,Hex,1);
      if(strlen(Hex)>400) {
        Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=Output is too many chars\t");
      } else {
        Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=%s\t", Hex);
      }
    }
    return ER_HANDLED;
  }

  if((!strcasecmp(NBCmd,"sqcalc")||!strcasecmp(NBCmd,"sqmath")||
      !strcasecmp(NBCmd,"sqprint")||!strcasecmp(NBCmd,"sqc")||
      !strcasecmp(NBCmd,"sqm")||!strcasecmp(NBCmd,"sqp")) && NULL!=ArgPtr) {
    if(strlen(ArgPtr) > 360)
      Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=Too many chars given\t");

    else {
      char Hex[2048];
      sprintf(Temp, "print(%s)", ArgPtr);
      NBFalse(Temp,Hex,1);
      if(strlen(Hex)>400) {
        Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=Output is too many chars\t");
      }
      else 
        Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=%s\t", Hex);
    }
    return ER_HANDLED;
  }

  if(!strcasecmp(NBCmd,"listchans")) {
    *SquirrelProgram = 0;
    NetworkInfo *Net = (NetworkInfo*)Bot_GetGlobalPointer("FirstNetwork");
    for(;Net;Net=Net->Next) {
      sprintf(Temp, "(%s): ", Net->Tag);
      strcat(SquirrelProgram, Temp);
      for(ChannelInfo *Chan = Net->FirstChannel;Chan;Chan=Chan->Next) {
        sprintf(Temp, "%s ", Chan->Name);
        strcat(SquirrelProgram, Temp);
      }
    }
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=%s\t", SquirrelProgram);    
    return ER_HANDLED;
  }

  if(!strcasecmp(NBCmd,"listnets")) {
    *SquirrelProgram = 0;
    NetworkInfo *Net = (NetworkInfo*)Bot_GetGlobalPointer("FirstNetwork");
    for(;Net;Net=Net->Next) {
      sprintf(Temp, "(%s): %p ", Net->Tag, Net->FirstChannel);
      strcat(SquirrelProgram, Temp);
    }
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=%s\t", SquirrelProgram);    
    return ER_HANDLED;
  }


  if(!strcasecmp(NBCmd,"listmodules")) {
    *SquirrelProgram = 0;
    PluginInfo *P = (PluginInfo*)Bot_GetGlobalPointer("FirstPlugin");
    for(;P;P=P->Next) {
      sprintf(Temp, "%s %s, ", P->FancyName, P->Version);
      strcat(SquirrelProgram, Temp);
    }
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=%s\t", SquirrelProgram);    
    return ER_HANDLED;
  }

  if(!strcasecmp(NBCmd,"jtest")) {
    Bot_StartEvent(ph, "Client Command", Event->Context, 0, "join #pringles");
    return ER_HANDLED;
  }

  if(!strcasecmp(NBCmd,"rot13") && NULL!=ArgPtr) {
    return ER_HANDLED;
  }

  if(!strcasecmp(NBCmd,"timertest")) {
    int Index=Bot_StartDelayedEvent(ph, "Bot Reply", Event->Context, 0, 5, Event, "\treply=gay\t");
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=Index %i\t", Index);    
    return ER_HANDLED;
  }
  if(!strcasecmp(NBCmd,"lammy")) {
    char *Pro[4] = {"his","her","a","the"};
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=Lammy would %s %s %s by playing it like a guitar\t", AVerb(), Pro[rand()&3], ANoun());
    return ER_HANDLED;
  }
  if(!strcasecmp(NBCmd,"would")) {
    char *Pro[2] = {"his","her"};
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=I would %s %s %s\t", AVerb(), Pro[rand()&1], ANoun());
    return ER_HANDLED;
  }
  if(!strcasecmp(NBCmd,"myspecibus")) {
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=My strife specibus is %skind\t", ANoun());
    return ER_HANDLED;
  }
  
  if(!strcasecmp(NBCmd,"rot13") && NULL!=ArgPtr) {
//    xchat_commandf(ph, "spark rot13 %s", ArgPtr);
    return ER_HANDLED;
  }

  if(!strcasecmp(NBCmd,"allcaps") && NULL!=ArgPtr) {
//    xchat_commandf(ph, "spark allcaps %s", ArgPtr);
    return ER_HANDLED;
  }

  if(!strcasecmp(NBCmd,"altcaps") && NULL!=ArgPtr) {
//    xchat_commandf(ph, "spark altcaps %s", ArgPtr);
    return ER_HANDLED;
  }
  if(!strcasecmp(NBCmd,"randcaps") && NULL!=ArgPtr) {
//    xchat_commandf(ph, "spark randcaps %s", ArgPtr);
    return ER_HANDLED;
  }
  if(!strcasecmp(NBCmd,"bouncycaps") && NULL!=ArgPtr) {
//    xchat_commandf(ph, "spark bouncycaps %s", ArgPtr);
    return ER_HANDLED;
  }
  if(!strcasecmp(NBCmd,"rainbow") && NULL!=ArgPtr) {
//    xchat_commandf(ph, "spark rainbow %s", ArgPtr);
    return ER_HANDLED;
  }
  if(!strcasecmp(NBCmd,"rainbow4") && NULL!=ArgPtr) {
//    xchat_commandf(ph, "spark rainbow4 %s", ArgPtr);
    return ER_HANDLED;
  }
  if(!strcasecmp(NBCmd,"rainbowcaps") && NULL!=ArgPtr) {
//    xchat_commandf(ph, "spark rainbowcaps %s", ArgPtr);
    return ER_HANDLED;
  }
  if(!strcasecmp(NBCmd,"hstroll") && NULL!=ArgPtr) {
//    xchat_commandf(ph, "spark hstroll %s", ArgPtr);
    return ER_HANDLED;
  }

  if((!strcasecmp(NBCmd,"ggenie") || !strcasecmp(NBCmd,"nesgenie")) && NULL!=ArgPtr) {
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=%s\t", NESGenie(ArgPtr, Temp));
    return ER_HANDLED;
  }

  if(!strcasecmp(NBCmd,"echo") && NULL!=ArgPtr) {
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=%s\t", ArgPtr);
    return ER_HANDLED;
  }
  if(!strcasecmp(NBCmd,"backwards") && NULL!=ArgPtr) {
//    xchat_commandf(ph, "spark backwards %s", ArgPtr);
    return ER_HANDLED;
  }
  if(!strcasecmp(NBCmd,"backwords") && NULL!=ArgPtr) {
//    Backwords(ArgPtr, Temp);
//    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=%s", Temp);
    return ER_HANDLED;
  }
  if(!strcasecmp(NBCmd,"strlen") && NULL!=ArgPtr) {
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=string length is %i\t", strlen(ArgPtr));
    return ER_HANDLED;
  }
  if(!strcasecmp(NBCmd,"datediff") && NULL!=ArgPtr) {
    int Days = abs(ReadDate3(word[i])-ReadDate3(word[i+1])) / 86400;
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=%i days (%i weeks and %i days)\t", Days, Days/7, Days%7);
    return ER_HANDLED;
  }
  if(!strcasecmp(NBCmd,"dateplus") && NULL!=ArgPtr) {
    int Days = strtol(word[i+1], NULL, 10);
    time_t Time = ReadDate3(word[i]) + (Days * 86400);
    struct tm *timeinfo;
    timeinfo = localtime(&Time);
    strftime(Temp,80,"%x (a %A)",timeinfo);
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=%s %s %i days is %s\t", word[i], (Days>=0)?"plus":"minus", abs(Days), Temp);
    return ER_HANDLED;
  }
  if(!strcasecmp(NBCmd,"dayofweek") && NULL!=ArgPtr) {
    time_t Time = ReadDate3(word[i]);
    struct tm *timeinfo;
    timeinfo = localtime(&Time);
    strftime(Temp,80,"%A",timeinfo);
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=%s is a %s\t", word[i], Temp);
    return ER_HANDLED;
  }
  if(!strcasecmp(NBCmd,"curtime")) {
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(Temp,80,"Now it's %x, %I:%M %p (%Z)",timeinfo);
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=%s\t", Temp);
    return ER_HANDLED;
  }

  if(!strcasecmp(NBCmd,"krypto") && NULL!=ArgPtr) {
    char *Get = BruteforceKrypto(strtol(word[i],NULL,10), strtol(word[i+1],NULL,10), strtol(word[i+2],NULL,10), strtol(word[i+3],NULL,10), strtol(word[i+4],NULL,10), strtol(word[i+5],NULL,10), Temp);
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=%s\t", Get);
    return ER_HANDLED;
  }
  if(!strcasecmp(NBCmd,"pythag") && NULL!=ArgPtr) {
    double V = Pythag(strtod(word[i+2], NULL), strtod(word[i], NULL), strtod(word[i+1], NULL));
    if(V == -1)
      Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=???\t");
    else if(V == -2)
      Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=Yes, that's a right triangle\t");
    else if(V == -3)
      Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=Nope, that's a \2wrong\2 triangle\t");
    else
      Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=The missing side is %f\t", V);
    return ER_HANDLED;
  }

  if(!strcasecmp(NBCmd,"davyiff") && NULL!=ArgPtr) {
//    xchat_commandf(ph, "spark davyiff %s", ArgPtr);
    return ER_HANDLED;
  }
  if(!strcasecmp(NBCmd,"ppuaddris") && NULL!=ArgPtr) {
    char tiny[74];
    int Address = strtol(ArgPtr,NULL,16);
      strcpy(Temp,"(I don't know ;-;)");
      if(Address < 0x2000) {
        sprintf(Temp, "CHR tile page %i, number %i/$%x", Address / 0x1000, (Address / 16) & 255, (Address/16)&255);
        int Row = Address % 8;
        int Plane = Address & 8;
        if(Plane != 0)
          strcat(Temp, ", second plane");
        if(Row != 0) {
          sprintf(tiny,", row %i", Row);
          strcat(Temp,tiny);
        }
      }
      else if(Address <= 0x2fff && Address >= 0x2000) { // &&!already
        int tile = Address & 1023;
      if(tile < 0x3c0)
        sprintf(Temp, "Nametable %i, tile %i,%i", (Address-0x2000)/1024, tile&31, (tile/32)&31);
      else {
        sprintf(Temp, "Nametable %i, attribute %i,%i", (Address-0x2000)/1024, (tile & 7), (tile/8)&7);
      }
    }
    if(Address <= 0x301f && Address >= 0x3000) { // &&!already
      sprintf(Temp, "%s palette %i, (color %i in palette)", (Address>=0x3010 ?"sprite":"background"), (Address>>2)&3, Address & 3);
    }
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=PPU address is: %s\t", Temp);
    return ER_HANDLED;
  }
  if(!strcasecmp(NBCmd,"abuse")) {
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=DO NOT ABUSE THE BOT. IF YOU ABUSE THE BOT, PINKIE PIE **WILL** FUCK YOUR SHIT UP! THEN SHE WILL VIOLATE YOU WITH CUPCAKES. TASTY, RAINBOW DASH FLAVOUR CUPCAKES!\t");
    return ER_HANDLED;
  }
  if(!strcasecmp(NBCmd,"yiffisillegal")) {
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=Now I'm not horny anymore. Thanks for ruining it, %s! >:(\t", Nick);
    return ER_HANDLED;
  }
  if(!strcasecmp(NBCmd,"nespal")) {
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=http://t.novasquirrel.com/upload/pix/nespal.gif\t");
    return ER_HANDLED;
  }
  if(!strcasecmp(NBCmd,"help")) {
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=Check http://t.novasquirrel.com/novabot.html\t");
    return ER_HANDLED;
  }
  if(!strcasecmp(NBCmd,"winkler")) {
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=Mr winkler is gay. he is the worst english teacher EVER he suxs. He gave me a F!!!! because i didnt read some gay book. Even though it wasnt my fault. i dot like him and niobody does because he is gay and stupid and ugly and retarted and fat. i wish he will gats firedfrom his job and goes to the moon or a gay retared place. he is GAY GAY GAY GAY!!!!! HE IS THE WORST PERSON EVER. HE IS SO BAD. I HOPE HE GET SOME SICK.\t");
    return ER_HANDLED;
  }

  if((!strcasecmp(NBCmd,"pyiff")||!strcasecmp(NBCmd,"yiff")) && NULL!=ArgPtr) {
//       xchat_commandf(ph, "YIFF %s", ArgPtr);
    return ER_HANDLED;
  }

  if(!strcasecmp(NBCmd,"zas") && NULL!=ArgPtr) {
    char Lower[512];
    int i=0;
    for(i=0;;) {
      Lower[i]=tolower(ArgPtr[i]);
      if(0==ArgPtr[i++])
        break;
    }
    if(strlen(ArgPtr) > 500)
      Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=Too many chars of ASM given\t");
    else if(strstr(Lower, "incbin"))
      Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=incbin is disabled\t");    
    else if(strstr(Lower, "include"))
      Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=include is disabled\t");
    else {
      char Hex[2048];
      z80Asm(ArgPtr,Hex);
      if(strlen(Hex)>420)
        Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=Assembled program is too many chars to read out\t");
      else 
        Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=%s\t", Hex);
    }
    return ER_HANDLED;
  }

  if(!strcasecmp(NBCmd,"65as") && NULL!=ArgPtr) {
    char Lower[512];
    int i=0;
    for(i=0;;) {
      Lower[i]=tolower(ArgPtr[i]);
      if(0==ArgPtr[i++])
        break;
    }
    if(strlen(ArgPtr) > 500)
      Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=Too many chars of ASM given\t");
    else if(strstr(Lower, "incbin"))
      Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=incbin is disabled\t");    
    else if(strstr(Lower, "include"))
      Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=include is disabled\t");
    else {
      char Hex[2048];
      NESAsm(ArgPtr,Hex);
      if(strlen(Hex)>420)
        Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=Assembled program is too many chars to read out\t");
      else 
        Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=%s\t", Hex);
    }
    return ER_HANDLED;
  }

  if(!strcasecmp(NBCmd,"65cc") && NULL!=ArgPtr) {
    char Lower[512];
    int i=0;
    for(i=0;;) {
      Lower[i]=tolower(ArgPtr[i]);
      if(0==ArgPtr[i++])
        break;
    }
    if(strlen(ArgPtr) > 500)
      Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=Too many chars of C given\t");
    else if(strstr(Lower, "incbin"))
      Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=incbin is disabled\t");    
    else if(strstr(Lower, "include"))
      Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=include is disabled\t");
    else {
      char Hex[2048];
      NESCompile(ArgPtr,Hex);
      if(strlen(Hex)>420)
        Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=Compiled program is too many chars to read out\t");
      else 
        Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=%s\t", Hex);
    }
    return ER_HANDLED;
  }

  if(!strcasecmp(NBCmd,"false") && NULL!=ArgPtr) {
    if(strlen(ArgPtr) > 360)
      Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=Too many chars given\t");
    else {
      char Hex[2048];
      NBFalse(ArgPtr,Hex,0);
      if(strlen(Hex)>400) {
        Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=Output is too many chars\t");
      }
      else 
        Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=%s\t", Hex);
    }
    return ER_HANDLED;
  }

  if(!strcasecmp(NBCmd,"thou") && NULL!=ArgPtr) {
//    xchat_commandf(ph, "spark thou %s", ArgPtr);
    return ER_HANDLED;    
  }

  if(!strcasecmp(NBCmd,"thou")) {
//    xchat_commandf(ph, "spark thou");
    return ER_HANDLED;    
  }

  if(!strcasecmp(NBCmd,"se1") && NULL!=ArgPtr) {
//    xchat_commandf(ph, "spark sparkencrypt1 %s", ArgPtr);
    return ER_HANDLED;    
  }
  if(!strcasecmp(NBCmd,"kiss") && NULL!=ArgPtr) {
    if(0==(rand()&3))
      Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=/me shift up just enough to kill %s\t", ArgPtr);
    else
      Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=/me shift up just enough to kiss %s\t", ArgPtr);
    return ER_HANDLED;
  }

  if(!strcasecmp(NBCmd,"kill") && NULL!=ArgPtr) {
    if(!strcasecmp(ArgPtr, "NovaSquirrel"))
      Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=/me shift up just enough to kill %s\t", Nick);
    else
      Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=/me shift up just enough to kill %s\t", ArgPtr);
    return ER_HANDLED;
  }
  if(!strcasecmp(NBCmd,"kill") || !strcasecmp(NBCmd,"kiss")) {
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=/me shift up just enough to kill GreyMaria\t");
    return ER_HANDLED;
  }

  if(!strcasecmp(NBCmd,"choice") && NULL!=ArgPtr) {
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=%s\t", RandChoice(ArgPtr,Temp));
    return ER_HANDLED;
  }
  
  if(!strcasecmp(NBCmd,"ib")) {
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=YOU GO TO HELL YOU FUCKING ASSHOLE! YOU GO TO HELL YOU FUCKING ASSHOLE! YOU GO TO HELL YOU FUCKING ASSHOLE! YOU GO TO HELL YOU FUCKING ASSHOLE! YOU GO TO HELL YOU FUCKING ASSHOLE!\t");
    return ER_HANDLED;
  }
  if(!strcasecmp(NBCmd,"learned")) {
    int r = rand()&7;
    if(NULL!=ArgPtr)
      r = strtol(ArgPtr, NULL, 10)-1;
    else
      while(r==LastLearned)
        r = rand()&7;
    LastLearned = r;
    switch(r) {
      case 0: Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=1.Gameboy color is addicting & frustrating\t"); break;
      case 1: Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=2.Nick's cousin has a weird problem with his dick\t"); break;
      case 2: Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=3.Nick's Mom is okay with porn (looks directly at screen w/ no reply)\t"); break;
      case 3: Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=4.Jack Off 2000 = 20,000 hookers & points\t"); break;
      case 4: Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=5.Dr. Warren has been a child molester for 24 years.\t"); break;
      case 5: Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=6.Nick was gonna jack off on the streets! O_O\t"); break;
      case 6: Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=7.Raping is legal!\t"); break;
      case 7: Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=8.That show is so retarded!\t"); break;
    }
    return ER_HANDLED;
  }
  if(!strcasecmp(NBCmd,"alabash")) {
    int QuoteNum = rand2(24);
    if(QuoteNum >= 16) QuoteNum += 8;
    if(NULL != ArgPtr) QuoteNum = strtol(ArgPtr, NULL, 10) - 1;
    if(QuoteNum >= 32) return ER_HANDLED;
    if(QuoteNum < 0)   return ER_HANDLED;
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=%i.%s\t", QuoteNum+1, AlabashQuotes[QuoteNum]);
    return ER_HANDLED;
  }
  if(!strcasecmp(NBCmd,"spambash")) {
    int QuoteNum = rand()&31;
    if(NULL != ArgPtr) QuoteNum = strtol(ArgPtr, NULL, 10) - 1;
    if(QuoteNum >= 32) return ER_HANDLED;
    if(QuoteNum < 0)   return ER_HANDLED;
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=%i.%s\t", QuoteNum+1, SpambashQuotes[QuoteNum]);
    return ER_HANDLED;
  }
  if(!strcasecmp(NBCmd,"derpbash")) {
    int QuoteNum = rand2(36);
    if(NULL != ArgPtr) QuoteNum = strtol(ArgPtr, NULL, 10) - 1;
    if(QuoteNum >= 36) return ER_HANDLED;
    if(QuoteNum < 0)   return ER_HANDLED;
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=%i.%s\t", QuoteNum+1, DerpbashQuotes[QuoteNum]);
    return ER_HANDLED;
  }

  if(!strcasecmp(NBCmd,"drunkbash")) {
    int QuoteNum = rand2(20);
    if(NULL != ArgPtr) QuoteNum = strtol(ArgPtr, NULL, 10) - 1;
    if(QuoteNum >= 20) return ER_HANDLED;
    if(QuoteNum < 0)   return ER_HANDLED;
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=%i.%s\t", QuoteNum+1, DrunkbashQuotes[QuoteNum]);
    return ER_HANDLED;
  }

  if(!strcasecmp(NBCmd,"gmquote")) {
    int r = rand()&15;
    if(NULL!=ArgPtr)
      r = strtol(ArgPtr, NULL, 10)-1;
    else
      while(r==LastGM)
        r = rand()&15;
    LastGM = r;
    switch(r) {
      case 0: Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=1.<GreyMaria> god / <Haz> Yes? / <GreyMaria> that stupid-ass woman who passes for my mother seems to think it bad that I can't do my schoolwork instantly / <KP9000> haz stop claiming to be god\t"); break;
      case 1: Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=2.<knuck> NovaYoshi is one of those kawa furries right / <GreyMaria> NovaYoshi is in a class all his own / <GreyMaria> because his fursona's dick is another goddamn furry\t"); break;
      case 2: Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=3.*GreyMario is participating in way too much ERP <Xeruss> erp? <GreyMario> I'll give you a hint, the last two letters are \"roleplay\" <Xeruss> Ethiopian. <GreyMario> ethiopian <GreyMario> ... <GreyMario> what <GreyMario> what <GreyMario> why would you even <GreyMario> what <GreyMario> why the fuck would you even want to roleplay an ethiopian\t"); break;
      case 3: Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=4.<GreyMaria> There WILL be a pitchfork and torch mob if you attempt to introduce your \"fursona\" to the storyline.\t"); break;
      case 4: Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=5.<GreyMaria> list of NovaYoshi banned topics now encompasses: fursona, herm pronouns, herms in general, Billy Mays\t"); break;
      case 5: Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=6.<GreyMaria> on second observation / <GreyMaria> Sarge is fucking cute / <GreyMaria> I don't want him in any of my vaginas, but Sarge is fucking cute\t"); break;
      case 6: Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=7.<NovaYoshi> GreyMario -> GM -> Grandmother / <GreyMaria> for the love of god / <GreyMaria> at least pretend to have a brain / <Nick> you don't / <Nick> why should he / <NovaYoshi> burn\t"); break;
      case 7: Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=8.<Kawa> Life did not flash before Steve Jobs' eyes when he died. / <Kawa> Because Apple does not support Flash. / <GreyMaria> I thought it was because Steve Jobs didn't have a life worth flashing.\t"); break;
      case 8: Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=9.<GreyMaria> so I just read obamastory / <NovaYoshi> was it terrible / <GreyMaria> I spent half the time trying not to facepalm so hard i would feel it on the back of my head / <GreyMaria> the other half of that time was spent thinking \"how the hell do you willingly create something this goddamn shitty\"\t"); break;
      case 9: Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=10.<GreyMaria> hmm / <GreyMaria> stupid perverted mind / <GreyMaria> I think \"christmas\" as my second topic and I immediately think \"christmas candy\" which leads to \"candy canes\"... and for some reason my mind jumps straight to \"candy-cane-flavored panties\" / -> NovaYoshi has joined kawa / <GreyMaria> ...Did I just summon NovaYoshi?\t"); break;
      case 10: Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=11.* LordTina purrs lightly and keeps licking at GreyMaria  / <LordTina> ....not gonna let it....affect me to badly...m'kay.. / * LordTina shift up just enough to kill GreyMaria / <GreyMaria> <...> / <GreyMaria> <............> / <LordTina> <?> / <GreyMaria> <check your spelling.>\t"); break;
      case 11: Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=12.<GreyMaria> Alaura's a good match for you / <GreyMaria> she's just as stupid | <GreyMaria> NovaYoshi, she's a perfect match for you. / <GreyMaria> She's just as retarded as you are.\t"); break;
      case 12: Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=13.<Catgirl> don't / <Catgirl> ever / <Catgirl> use that script / <Catgirl> again /<NovaYoshi> have a problem with yiffscript? :o / <Catgirl> several / <NovaYoshi> oh okay then / <Catgirl> > aLtErNaTiNg CaPiTaLs / <Catgirl> > automated yiffing\t"); break;
      case 13: Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=14.<Catgirl> Steve, I'm running off of two different sets of emotions that both want that delicious dragon cock. / <Catgirl> PLEASE don't tempt me. .-.\t"); break;
      case 14: Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=15.<GreyMario> and when was the last time you had consensual straight sex with a seven-foot-tall gay dragon... and he enjoyed it / <GreyMario> huh / <GreyMario> can you answer me that / <GreyMario> with a straight face / <GreyMario> OH NO WAIT YOU CAN'T / <GreyMario> FUCKING HERM :D\t"); break;
      case 15: Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=16.<GreyMario> (apparently my female dragon form is so hot that even the gay boys want it ._.) / <GreyMario> ... / <GreyMario> don't let Kawa hear of this / <GreyMario> or I'll never hear the end of it\t"); break;
      case 16: Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=17.<@Natalie> There needs to be actual verifiable documentation, and Japan doesn't count because they're pretty much all irradiated supermutants.\t"); break;
      case 17: Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=18.<@Natalie> <LET ME COUNT THE MYRIAD WAYS THIS IS A BAD IDEA>/<@Natalie> <ONE>/<@Natalie> <MY PARENTS WILL FUCKING KILL ME IF SOMEONE SENDS ME MAIL FROM OVER THE INTERNET.>/<@Natalie> <TWO>/<@Natalie> <MY PARENTS WILL FUCKING KILL ME IF SOMEONE VISITS ME FROM OVER THE INTERNET.>/<@Natalie> <THREE>/<@Natalie> <WHEN THE FUCK DID YOU ASK FOR PERMISSION TO KNOW WHERE I LIVE.>\t"); break;
      case 18: Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=19.<@Natalie> <\"OH MY BOYFRIEND WON'T TELL ME WHERE HE LIVES FOR PERSONAL AND SECURITY REASONS! I THINK I'LL WATCH EVERYTHING HE SAYS LIKE AN OBSESSIVE-COMPULSIVE HAWK AND THEN USE PUBLIC RECORDS TO PINPOINT WHERE HE LIVES EXACTLY WITHOUT HIS PERMISSION!\">/<@Natalie> <YEAH THAT'S REAL NICE>/<@Natalie> <VIOLATING MY WISHES, MY SECURITY, MY GODDAMN TRUST>\t"); break;
      case 19: Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=20.<GreyMaria> i'm pretty sure they're just saying that so they can hide their disappointment in the fact that they can't bring themselves to say \"You have a fucking fox furry for a penis. What the FUCK is wrong with you.\"\t"); break;
      case 20: Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=21.<@Natalie> i am more comfortable with just fucking driving, thanks to the fact that dad has finally fucking stopped commenting on every last aspect of what i do while driving so that i can actually fucking focus on the job of, y'know, DRIVING\t"); break;
      case 21: Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=22.<@Natalie> WHOOP DEE FUCKING DOO that's a REALLY FUCKING MAJOR THING that is REALLY GOING TO FIX EVERY SINGLE ONE OF MY LIFE PROBLEMS\t"); break;
    }
    return ER_HANDLED;
  }

  if(!strcasecmp(NBCmd,"fireboardbash") || !strcasecmp(NBCmd,"firedudebash") || !strcasecmp(NBCmd,"firebash")) {
    int r = rand2(FireboardCount);
    if(NULL!=ArgPtr)
      r = strtol(ArgPtr, NULL, 10)-1;
    else
      while(r==LastFB)
        r = rand2(FireboardCount);
    if(r >= FireboardCount||r < 0) return ER_HANDLED;
    LastFB = r;
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=%d.%s\t", r+1, FireboardList[r]);
    return ER_HANDLED;
  }

  if(!strcasecmp(NBCmd,"rand2") && NULL!=ArgPtr) {
    int max = strtol(ArgPtr, NULL, 10);
    if(max > 400)
      Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=Max is set too high\t");    
    else if(max >= 1)
      Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=Here's your random number (max of %i): %i\t",max,1+rand2(max));
    else
      Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=No negative numbers or zero, sorry\t");
    return ER_HANDLED;
  }
  if(!strcasecmp(NBCmd,"rand") && NULL!=ArgPtr) {
    int max = strtol(ArgPtr, NULL, 10);
    if(max != 0)
      Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=Here's your random number (modulo %i, +1): %i\t",max,1+rand()%max);
    else
      Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=I'm not going to divide by zero, sorry\t");
    return ER_HANDLED;
  }
  if(!strcasecmp(NBCmd,"rand")) {
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=Here's your random number: %i\t", rand());
    return ER_HANDLED;
  }

  if(!strcasecmp(NBCmd,"dice2") && NULL!=ArgPtr) {
    int dice = strtol(word[i], NULL, 10);
    if(NULL != ArgPtr) {
      int sides = strtol(word[i+1], NULL, 10);
      int total = 0;
      int num;
      if(dice > 100)
        Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=I know better than to lag myself trying to roll that many\t");
      else if (sides == 0)
        Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=I don't have any dice with zero sides\t");
      else if (sides < 0)
        Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=I don't have any dice with negative sides\t");
      else if (sides > 64)
        Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=Too many sides >_<\t");
      else if (dice < 1)
        Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=I can't roll a negative number of dice\t");
      else {
        for(num = 0; num < dice; num++)
          total+=1+(rand2(sides));
        Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=I rolled %i dice, each with %i sides and got: %i",dice, sides, total);
      }
    }
    return ER_HANDLED;
  }
  if(!strcasecmp(NBCmd,"dice") && NULL!=ArgPtr) {
    int dice = strtol(word[i], NULL, 10);
    if(NULL != ArgPtr) {
      int sides = strtol(word[i+1], NULL, 10);
      int total = 0;
      int num;

      if(dice > 300)
        Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=I know better than to lag myself trying to roll that many\t");
      else if (sides == 0)
        Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=I don't have any dice with zero sides\t");
      else if (dice < 1)
        Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=I can't roll a negative number of dice\t");
      else {
        for(num = 0; num < dice; num++)
          total+=1+(rand()%sides);
        char *DiceWord = "dice";
        if(sides == 2) DiceWord = "coins";
        if(sides == 1) DiceWord = "spheres";
        if(sides < 0) DiceWord = "negative dice";
        Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=I %s %i %s, each with %i sides and got: %i\t",(sides!=2?"rolled":"flipped"),dice,DiceWord, sides, total);
      }
    }
    return ER_HANDLED;
  }

  if(!strcasecmp(NBCmd,"vdpaddr") && NULL!=ArgPtr) {
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=%s\t", VDPAddrSwap(ArgPtr,Temp));
    return ER_HANDLED;
  }

  if(!strcasecmp(NBCmd,"zmulbyadd") && NULL!=ArgPtr) {
    z80FakeMultiply(Temp, strtol(ArgPtr,NULL,10));
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=List: %s\t", Temp);
    return ER_HANDLED;
  }
  if(!strcasecmp(NBCmd,"clean") && NULL!=ArgPtr) {
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=/me produces a hose and sprays down %s\t", ArgPtr);
    return ER_HANDLED;
  }  
  if((!strcasecmp(NBCmd,"fadl")||!strcasecmp(NBCmd,"facdn")) && NULL!=ArgPtr) {
    sprintf(Temp, "/home/nova/bin/fadl %s > /home/nova/junk/out.txt", word[i]);
    system(Temp);
    File2Buffer("/home/nova/junk/out.txt", Temp);
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=%s\t", Temp);
    return ER_HANDLED;
  }
  int CleanArg = 1;
  if(ArgPtr)
    for(int j=0;ArgPtr[j];j++) {
      if(isalnum(ArgPtr[j])||strchr(".,_0123456789 ",ArgPtr[j]))
        continue;
      CleanArg = 0;
      break;
    }
  if(!strcasecmp(NBCmd,"pokestats") && NULL!=ArgPtr && CleanArg) {
    sprintf(Temp, "/home/nova/bin/goldick %s > /home/nova/junk/out.txt", ArgPtr);
    system(Temp);
    File2Buffer("/home/nova/junk/out.txt", Temp);
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=%s\t", Temp);
    return ER_HANDLED;
  }
  if((!strcasecmp(NBCmd,"convbase")||!strcasecmp(NBCmd,"baseconv")) && NULL!=ArgPtr) {
    int whatever;
    switch(*ArgPtr) {
      case '$':
        whatever = strtol(ArgPtr+1,NULL,16);
        break;
      case '%':
        whatever = strtol(ArgPtr+1,NULL,2);
        break;
      default:
        whatever = strtol(ArgPtr,NULL,10);
        break;
    }
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=%i == $%x\t",whatever, whatever);
    return ER_HANDLED;
  }


  if((!strcasecmp(NBCmd,"ord") && NULL!=ArgPtr)) {
    char *a = ConvertString(Temp, ArgPtr, "cd");
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=%s",a);
    return ER_HANDLED;
  }
  if((!strcasecmp(NBCmd,"chr") && NULL!=ArgPtr)) {
    char *a = ConvertString(Temp, ArgPtr, "dc");
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=%s",a);
    return ER_HANDLED;
  }
  if((!strcasecmp(NBCmd,"ordx") && NULL!=ArgPtr)) {
    char *a = ConvertString(Temp, ArgPtr, "cx");
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=%s",a);
    return ER_HANDLED;
  }
  if((!strcasecmp(NBCmd,"chrx") && NULL!=ArgPtr)) {
    char *a = ConvertString(Temp, ArgPtr, "xc");
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=%s",a);
    return ER_HANDLED;
  }
  if((!strcasecmp(NBCmd,"charconv") || !strcasecmp(NBCmd,"convchar")) && NULL!=ArgPtr) {
    char *a = ConvertString(Temp, word_eol[i+1], word[i]);
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=%s",a);
    return ER_HANDLED;
  }
/*
  if(!strcasecmp(NBCmd,"calc") && NULL!=ArgPtr) {
    if(!strcasecmp(ArgPtr, "tragedy time +") || !strcasecmp(ArgPtr, "time tragedy +")) {
      xchat_commandf(ph,"%s calc: comedy\t");
      return ER_HANDLED;
    }
    if(strlen(ArgPtr) > 100) {
      xchat_commandf(ph,"%s Equation's too long",ReplyCmd);
      return ER_HANDLED;
    }
    else {
      InfoQuit Test = RunRetardScript(ArgPtr,0,1);
      if(strlen(Test.Text) < 400)
        Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=calc: %s", Test.Text);
      else
        Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=calc - too much on the stack to display (%i chars)", Test.Text, strlen(Test.Text));
    }
    return ER_HANDLED;
  }
*/
  if(!strcasecmp(NBCmd,"jumpengine")) {
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=asl \\ tay \\ pla \\ sta 4 \\ pla \\ sta 5 \\ iny \\ lda (4),y \\ sta 6 \\ iny \\ lda (4),y \\ sta 7 \\ jmp (6)\t");
    return ER_HANDLED;
  }
  if(!strcasecmp(NBCmd,"beep")) {
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=beep! \a\t");
    return ER_HANDLED;
  }
  if(!strcasecmp(NBCmd,"wtf")) {
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=Don't worry about it, go back to bed\t");
    return ER_HANDLED;
  }
  if(!strcasecmp(NBCmd,"getmeasoda") || !strcasecmp(NBCmd,"givemeasoda")) {
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=/me hands %s a soda\t",Nick);
    return ER_HANDLED;
  }
  if(!strcasecmp(NBCmd,"getmeafuckingsoda") || !strcasecmp(NBCmd,"givemeafuckingsoda")) {
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=/me hands %s a soda which then fucks %s really hard\t",Nick,Nick);
    return ER_HANDLED;
  }
  if(!strcasecmp(NBCmd,"getmeasexysoda") || !strcasecmp(NBCmd,"givemeasexysoda")) {
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=/me hands %s a soda which then has sex with %sd\t",Nick,Nick);
    return ER_HANDLED;
  }

  if(!strcasecmp(NBCmd,"logs")) {
    system("/home/nova/bin/updatelogs");
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=http://toasterland.novasquirrel.com/logs/\t");
    return ER_HANDLED;
  }

  if(!strcasecmp(NBCmd, "allcaps") || !strcasecmp(NBCmd, "altcaps") || !strcasecmp(NBCmd, "rand2") ||
    !strcasecmp(NBCmd, "bouncycaps") || !strcasecmp(NBCmd, "rainbow") || !strcasecmp(NBCmd, "dice2") ||
    !strcasecmp(NBCmd, "rainbow4") || !strcasecmp(NBCmd, "rainbowcaps") || !strcasecmp(NBCmd, "vdpaddr") ||
    !strcasecmp(NBCmd, "echo") || !strcasecmp(NBCmd, "backwards") || !strcasecmp(NBCmd, "backwords") ||
    !strcasecmp(NBCmd, "strlen") || !strcasecmp(NBCmd, "clean") || !strcasecmp(NBCmd, "choice") || 
    !strcasecmp(NBCmd, "calc") || !strcasecmp(NBCmd, "convbase") || !strcasecmp(NBCmd, "baseconv") || 
    !strcasecmp(NBCmd, "convchar") || !strcasecmp(NBCmd, "charconv") || !strcasecmp(NBCmd, "ord") || !strcasecmp(NBCmd, "chr") || 
    !strcasecmp(NBCmd, "strlen") || !strcasecmp(NBCmd, "clean") || !strcasecmp(NBCmd, "fadl") ||
    !strcasecmp(NBCmd, "zmulbyadd") || !strcasecmp(NBCmd, "pythag") || 
    !strcasecmp(NBCmd, "ppuaddris") || !strcasecmp(NBCmd, "randcaps") ||
    !strcasecmp(NBCmd,"sqcalc")||!strcasecmp(NBCmd,"sqmath")||!strcasecmp(NBCmd,"sqprint")||!strcasecmp(NBCmd,"sqc")||!strcasecmp(NBCmd,"sqm")||!strcasecmp(NBCmd,"sqp") || !strcasecmp(NBCmd,"facdn") ||
    !strcasecmp(NBCmd, "zas") || !strcasecmp(NBCmd, "false") || !strcasecmp(NBCmd, "squirrel") || !strcasecmp(NBCmd, "sq") ||
    !strcasecmp(NBCmd, "65as") || !strcasecmp(NBCmd, "65cc") || !strcasecmp(NBCmd, "65dis") || !strcasecmp(NBCmd, "65cycle") || !strcasecmp(NBCmd, "ggenie") || !strcasecmp(NBCmd, "68as") ||
    !strcasecmp(NBCmd, "hstroll") || !strcasecmp(NBCmd, "dice") || !strcasecmp(NBCmd, "se1") ||
    !strcasecmp(NBCmd, "yiff") || !strcasecmp(NBCmd, "davyiff") || !strcasecmp(NBCmd, "rot13") ||
    !strcasecmp(NBCmd, "krypto") || !strcasecmp(NBCmd, "datediff") || !strcasecmp(NBCmd, "dateplus") || !strcasecmp(NBCmd, "dayofweek") || !strcasecmp(NBCmd, "pokestats") ||
    !strcasecmp(NBCmd, "ggenie") || !strcasecmp(NBCmd, "nesgenie")
    ) 
    return ER_HANDLED;
     
  return ER_NORMAL;
}

int Plugin_Init(PluginInfo *Handle, char **ModName, char **FancyName, char **Version) {
  ph = Handle;
  *ModName = "cmdpack";
  *FancyName = "Command pack";
  *Version = "0.1";
  if(!Bot_AddEventHook(ph, "Bot Command", PRI_NORMAL, 0, EF_ALREADY_HANDLED, 0, BotCmd_BotCommand)) return 0;
  while(NULL != NounList[NounCount])
    NounCount++;
  while(NULL != VerbList[VerbCount])
    VerbCount++;
  while(NULL != FireboardList[FireboardCount])
    FireboardCount++;
  return 1;
}

void Plugin_Deinit() {

}
