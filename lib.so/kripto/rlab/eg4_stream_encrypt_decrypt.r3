//
//
//
rfile libkripto.so

iv  = int(zeros(1,8));
key = [1L,int(zeros(1,15))];
r = 8;
msg = int(zeros(1,64));

r_blcyf = ["chacha", "keccak800", "keccak1600", "rc4", "salsa20"];

"Original message (hex): 0x" + num2str(msg,"%02x", "") + "\n" ?
for (blcyp in r_blcyf)
{
  colors("green");
  blcyp + ":\n"?
  colors();
  s = encrypt(blcyp,msg,r,key,iv);
  "Encoded message (hex): 0x" + num2str(s,"%02x", "") + "\n" ?
  m = decrypt(blcyp,s,r,key,iv);
  "Decoded message (hex): 0x" + num2str(m,"%02x", "") + "\n" ?
}

r_blk = ["aes", "anubis", "aria", "blowfish", "craxs", "des", "idea", "khazad", "lea", "noekeon", ...
    "rc2", "rc5", "rc6", "rectangle", "rijndael128", "rijndael256", "safer", "saferpp", "safersk", ...
    "seed", "serpent", "shacal2", "simon64", "simon128", "sm4", "speck32", "speck64", "speck128", ...
    "tea", "threefish256", "threefish512", "threefish1024", "traxl", "traxm", "twofish", "xtea"];
//r_cyf = ["cbc", "cfb", "ecb", "ofb"];
r_cyf = ["cbc", "ecb"];

for (r_b in r_blk)
{
  for (r_c in r_cyf)
  {
    blcyp = r_b + ":" + r_c;
    colors("green");
    blcyp + ":\n"?
        colors();
    s = encrypt(blcyp,msg,r,key,iv);
    "Encoded message (hex): 0x" + num2str(s,"%02x", "") + "\n" ?
        m = decrypt(blcyp,s,r,key,iv);
    "Decoded message (hex): 0x" + num2str(m,"%02x", "") + "\n" ?
  }
}
stop()



blcyp = "khazad:ecb";
s = encrypt(blcyp,msg,r,key,iv);
blcyp + ": Encoded message (hex): 0x" + num2str(s,"%02x", "") + "\n" ?
m = decrypt(blcyp,s,r,key,iv);
blcyp + ": Decoded message (hex): 0x" + num2str(m,"%02x", "") + "\n" ?

blcyp = "simon128:ecb";
s = encrypt(blcyp,msg,r,key,iv);
blcyp + ": Encoded message (hex): 0x" + num2str(s,"%02x", "") + "\n" ?
m = decrypt(blcyp,s,r,key,iv);
blcyp + ": Decoded message (hex): 0x" + num2str(m,"%02x", "") + "\n" ?

blcyp = "twofish:ecb";
s = encrypt(blcyp,msg,r,key,iv);
blcyp + ": Encoded message (hex): 0x" + num2str(s,"%02x", "") + "\n" ?
m = decrypt(blcyp,s,r,key,iv);
blcyp + ": Decoded message (hex): 0x" + num2str(m,"%02x", "") + "\n" ?







