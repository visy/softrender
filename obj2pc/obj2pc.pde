import processing.opengl.*;

// square_obj v.02 elout de kok | 28 september 2003 | http://www.xs4all.nl/~elout/
// this needs a texture-image in your data diretory
// and reads an 3D .obj  file 
// obj format; http://www.google.com/search?q=obj+format&ie=UTF-8&oe=UTF-8&hl=nl&lr=
// used the free max2obj plug-in http://www.habware.at/duck4.htm
// for creating the obj file.
//
// processing - version.0062

PImage tex1;
PImage brush;
PImage brushM;

//texture width/height
int texw=512;
int texh=512;

//------mouse & stuff
int mx,my,sx=0,sy=-100, middenx,middeny;
float xangle,yangle,zangle;

float zpos=470;

int no_vertex;
float [] point_buff = new float[15000];
int pointbuffcounter=0;

int no_texvertex;
int [] pointtex_buff = new int[15000];

int pointtexbuffcounter=0;

float [] pointnormal_buff =  new float[15000];
int normalcounter=0;



int no_faces;
int [] face_buff = new int [15000];
int [] texface_buff = new int [15000];
int facebuffcounter=0;

// my personal textbuffer -> now max 5000 lines
int my_textbuffersize=15000;
String [] my_textbuffer = new String[my_textbuffersize];


void setup()
{
  size(800,600,OPENGL);
  hint(ENABLE_DEPTH_SORT);
  noStroke();
  cursor(ARROW);
          
  middenx=width/2;
  middeny=height/2;
    
  //load the texture image - from your data folder 
  //tex1 = loadImage("wierdbox.gif");
  tex1 = loadImage("data/mara2.jpg");
  brush = loadImage("data/brushmask.jpg");
  brushM = loadImage("data/brushmask.jpg");
 
  brush.mask(brushM);
 
  //load a textfile - from your data directory
  //String lines[] = loadStrings("wierdbox.obj");
  String lines[] = loadStrings("data/sikiotri.obj");

  //println("------------------------------------------------");
  //println("--------------------------start-----------------");
  //check the 3D data that`s loaded
      int firstblood=0;
            println("vec3 normals[236] = {");

  for (int i= 0; i < lines.length; i++)
  {
   // println(lines[i]);
    if (lines[i].equals(""))
    {
      //do nothing - an empty line
    }
    else
    {

      String llist[] = split(lines[i],' ');  // '\t'
      for (int j= 0; j < llist.length; j++)
      {
        // load the vertex information
        if (llist[j].equals("v"))
        {
          if (firstblood == 0) {
            firstblood++;
           // println("const Vect3D_f16 cube_face_norm[74] = {");
            };
          float floatlist1[] = float(split(llist[j+2],' ')); 
          point_buff[pointbuffcounter]=floatlist1[0];
          float floatlist2[] = float(split(llist[j+3],' ')); 
          point_buff[pointbuffcounter+1]=floatlist2[0];
          float floatlist3[] = float(split(llist[j+4],' ')); 
          point_buff[pointbuffcounter+2]=floatlist3[0];
          
/*
          if (lines[i+2].equals("")) {
            println("  {FIX16("+point_buff[pointbuffcounter]+"), FIX16("+point_buff[pointbuffcounter+1]+"), FIX16("+point_buff[pointbuffcounter+2]+")}};");
            println("  //"+lines[i+1]);
            firstblood=0;
          } else {
            println("  {FIX16("+point_buff[pointbuffcounter]+"), FIX16("+point_buff[pointbuffcounter+1]+"), FIX16("+point_buff[pointbuffcounter+2]+")},");
          }
*/
          pointbuffcounter=pointbuffcounter+3;         

        }
        //load texture point information - and convert it to the texture width/height
        if (llist[j].equals("vt"))
        {          
          float floatlist4[] = float(split(llist[j+1],' ')); 
          pointtex_buff[pointtexbuffcounter]=(int) (floatlist4[0]*(float)texw); //u
          float floatlist5[] = float(split(llist[j+2],' ')); 
          pointtex_buff[pointtexbuffcounter+1]=(int) (texh-(floatlist5[0]*(float)texh)); //v - texture seems to flipped vertical thats why 'texh-'
          float floatlist6[] = float(split(llist[j+3],' ')); 
          //pointtex_buff[pointtexbuffcounter+2]=(int) floatlist6[0]; //not used yet
          //println("u "+pointtex_buff[pointtexbuffcounter]+" v "+pointtex_buff[pointtexbuffcounter+1]+" w "+pointtex_buff[pointtexbuffcounter+2]);
          pointtexbuffcounter=pointtexbuffcounter+3;         
        }

        //load face normal information
        if (llist[j].equals("vn"))
        {          
          float floatlist7[] = float(split(llist[j+1],' ')); 
          pointnormal_buff[normalcounter]=floatlist7[0];
          float floatlist8[] = float(split(llist[j+2],' ')); 
          pointnormal_buff[normalcounter+1]=floatlist8[0];
          float floatlist9[] = float(split(llist[j+3],' ')); 
          pointnormal_buff[normalcounter+2]=floatlist9[0];
          //println("  {intToFix16("+int(pointnormal_buff[normalcounter])+"), intToFix16("+int(pointnormal_buff[normalcounter+1])+"), intToFix16("+int(pointnormal_buff[normalcounter+2])+")},");
          //println("    {FIX16("+pointnormal_buff[normalcounter]+"), FIX16("+pointnormal_buff[normalcounter+1]+"), FIX16("+pointnormal_buff[normalcounter+2]+")},");
          //println("  {FIX16("+point_buff[pointbuffcounter]+"), FIX16("+point_buff[pointbuffcounter+1]+"), FIX16("+point_buff[pointbuffcounter+2]+")},");
          normalcounter=normalcounter+3;         
        }


        //load face information - note .obj points starts at 1 not 0;
        if (llist[j].equals("f"))
        {
          
 
          int [] normalindex = new int[4];
          int intlist1[] = int(split(llist[j+1],'/'));
          face_buff[facebuffcounter]=intlist1[0]-1;
          //texface_buff[facebuffcounter]=intlist1[1]-1;
          normalindex[0] = intlist1[1]-1;
          int intlist2[] = int(split(llist[j+2],'/')); 
          face_buff[facebuffcounter+1]=intlist2[0]-1;
//          texface_buff[facebuffcounter+1]=intlist2[1]-1;
          normalindex[1] = intlist2[1]-1;
          int intlist3[] = int(split(llist[j+3],'/')); 
          face_buff[facebuffcounter+2]=intlist3[0]-1;
//          texface_buff[facebuffcounter+2]=intlist3[1]-1;
          normalindex[2] = intlist3[1]-1;
          int intlist4[] = int(split(llist[j+4],'/')); 
          face_buff[facebuffcounter+3]=intlist4[0]-1;
//          texface_buff[facebuffcounter+3]=intlist4[1]-1;
//          normalindex[3] = intlist4[1]-1;
    
          float xnorm = 0;
          float ynorm = 0;
          float znorm = 0;
    
          float sum = 0;
          
            for(int e=0;e<3;e++){
            for(int a=0;a<4;a++){
            int asa = normalindex[a];
            sum += pointnormal_buff[3*asa+e];
            }
            sum = sum/4;
            sum = round(sum,2);
            if (e == 0) xnorm = sum;
            if (e == 1) ynorm = sum;
            if (e == 2) znorm = sum;
            
            
            sum=0;
          }

          

          if (lines[i+2].equals("")) {
            println(" {"+xnorm+", "+ynorm+", "+znorm+"}};");
            println("  //"+lines[i+1]);
            firstblood=0;
          } else {
            println(" {"+xnorm+", "+ynorm+", "+znorm+"},");
          }


          //println("a "+face_buff[facebuffcounter]+" b "+face_buff[facebuffcounter+1]+" c "+face_buff[facebuffcounter+2]+" d "+face_buff[facebuffcounter+3]);
          //println("ta "+texface_buff[facebuffcounter]+" tb "+texface_buff[facebuffcounter+1]+" tc "+texface_buff[facebuffcounter+2]);
          facebuffcounter=facebuffcounter+4;   
          no_faces++;  
        }       
      }
    }
  }
  //println("total vertex "+pointbuffcounter/3);
  //println("total texture vertex "+pointtexbuffcounter/3);
  //println("total faces "+facebuffcounter/3);

    println();
    println("vec3 vertices["+pointbuffcounter/3+"] = {");

    for (int a=0;a<pointbuffcounter;a+=3){
      int t=1;
          //if (lines[i+2].equals("")) {
          if (a < (pointbuffcounter-3)) {
            
            println("  {"+int(t*point_buff[a])+", "+int(t*point_buff[a+1])+", "+int(t*point_buff[a+2])+"},");
            //firstblood=0;
          } else {
            println("  {"+int(t*point_buff[a])+", "+int(t*point_buff[a+1])+", "+int(t*point_buff[a+2])+"}};");
          }
    }
            println("  // "+pointbuffcounter/3+" vertices");

    println();
    println("vec3 faces["+no_faces+"] = {");

    for (int a=0;a<4*(no_faces);a+=4){
          if (a < 4*(no_faces-1)) println("  {"+face_buff[a]+", "+face_buff[a+1]+", "+face_buff[a+2]+"},");
          else println("  {"+face_buff[a]+", "+face_buff[a+1]+", "+face_buff[a+2]+"}};");
    }
           // print("  };");
            println("  // "+no_faces+" polygons");

}
float mill =0;
void draw()
{
  
  background(128,128,128);

//  brush.alpha(brush); 

      
  //zoom in/out using arrow keys
  if(keyPressed)
  {
    if (key == UP){ zpos=zpos+20.0f;}
    if (key == DOWN){ zpos=zpos-20.0f;}
  }

  // set the position to the middle of your screen 
  translate(middenx, middeny+14, -20+zpos);

  
  //check mouse for rotation
  mx=mouseX;
  my=mouseY;
  if ( mx > middenx){sx=0-((middenx-mx)/2);}
  else{sx=((mx-middenx)/2);}
  if ( my > middeny){sy=0-((middeny-my)/2);}
  else{sy=((my-middeny)/2);}
  //xangle= xangle+(sy/2000.0f);
  //yangle= yangle+((sx-sy)/2000.0f);
  zangle= zangle+(sx/400.0f);
  //rotateX(xangle);
  //rotateX(1.6+(-sy/80.0));
  rotateX(1.6);

 mill = millis();
  rotateZ(.001*mill);
  //rotateZ(1.0+(sx/60.0));

  //draw the model  
 // beginShape(TRIANGLES);
  //texture(tex1);
  for (int i=0;i<no_faces;i+=1)
  {

  pushMatrix();

  

//    point_buff[ (face_buff[(i*4) + 3])*3 ],
  //  point_buff[ ((face_buff[(i*4) + 3])*3) + 1],
   // point_buff[ ((face_buff[(i*4) + 3])*3) + 2]


 
  /*
    pointtex_buff[ (texface_buff[i*3])*3],
    pointtex_buff[ ((texface_buff[i*3])*3) + 1],
    
    pointtex_buff[ (texface_buff[(i*3) + 1])*3 ],
    pointtex_buff[ ((texface_buff[(i*3) + 1])*3) + 1],

    pointtex_buff[ (texface_buff[(i*3) + 2])*3 ],
    pointtex_buff[ ((texface_buff[(i*3) + 2])*3) + 1]        
    */
  

   popMatrix();

  }
  //endShape();
 
  //put in a small delay, so the system can handle other tasks as well
  delay(15);
}

void drawtri3d(float x1,float  y1,float  z1, float  x2,float  y2,float  z2, float  x3,float  y3,float  z3,float  x4,float  y4,float  z4)
{
  
  scale(16);
  beginShape(QUAD);
  fill(255*abs((x1+y1+z1)));
  vertex(x1, y1, z1);
  vertex(x2, y2, z2);
  vertex(x3, y3, z3);
  vertex(x4, y4, z4);
  endShape();
 

  
  
}

float round(float number, float decimal) {
  return (float)(round((number*pow(10, decimal))))/pow(10, decimal);
} 
