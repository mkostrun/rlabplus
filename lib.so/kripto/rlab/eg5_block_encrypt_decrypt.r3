rfile libkripto.so

twk = int(zeros(1,8));
key = [1L,int(zeros(1,15))];
r = 0;
msg = int(zeros(1,64));
iv = [];

r_blk = ["aes", "anubis", "aria", "blowfish", "craxs", "des", "idea", "khazad", "lea", "noekeon", ...
    "rc2", "rc5", "rc6", "rectangle", "rijndael128", "rijndael256", "safer", "saferpp", "safersk", ...
    "seed", "serpent", "shacal2", "simon64", "simon128", "sm4", "speck32", "speck64", "speck128", ...
    "tea", "threefish256", "threefish512", "threefish1024", "traxl", "traxm", "twofish", "xtea"];

for (blcyp in r_blk[2])
{
  colors("green");
  blcyp + ":\n"?
  colors();
  "Message (hex): 0x" + num2str(msg,"%02x", "") + "\n" ?
  s = block.encrypt(blcyp,msg,r,key,iv);
  "Encoded block (hex): 0x" + num2str(s,"%02x", "") + "\n" ?
  m = block.decrypt(blcyp,s,r,key,iv);
  "Decoded block (hex): 0x" + num2str(m,"%02x", "") + "\n" ?
}










