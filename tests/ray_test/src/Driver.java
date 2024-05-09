import java.io.*;

public class Driver {
    public static class Vector {
        public double x, y, z;
        public Vector(double x, double y, double z) {
            this.x = x;
            this.y = y;
            this.z = z;
        }
        public Vector add(Vector v) {return new Vector(x + v.x, y + v.y, z + v.z);}
        public Vector sub(Vector v) {return new Vector(x - v.x, y - v.y, z - v.z);}
        public Vector mult(double d) {return new Vector(x * d, y * d, z * d);}
        public Vector div(double d) {return new Vector(x / d, y / d, z / d);}
        public Vector normalize() {
            double m = Math.sqrt(x*x+y*y+z*z);
            return new Vector(x / m, y / m, z / m);
        }
        public double dot(Vector v) {return (x*v.x+y*v.y+z*v.z);}
    }


    public static class Sphere {
        // sphere takes in a center point c, and a radius r
        public Vector c;
        public double r;
        public Vector col;
        public Sphere(Vector c, double r, Vector col) {this.c = c;this.r = r;this.col = col;}
        public Vector getNormal(Vector p) {return (p.sub(c)).div(r);}
        // intersects function returns true if this sphere intersects with the ray passed in
        public boolean intersects(Ray ray) {
            // if the discriminant of the intersection formula is a negative
            // or very small number, it does not intersect.
            Vector p = ray.o.sub(c);
            double b = 2*p.dot(ray.d);
            double c = p.dot(p)-(r*r);
            double disc = b*b - 4*c;
            if (disc < 0.0001) {return false;}
            return true;
        }
        public double intersection(Ray ray) {
            Vector p = ray.o.sub(c);
            double b = 2*p.dot(ray.d);
            double c = p.dot(p)-(r*r);
            double disc = Math.sqrt(b*b - 4*c);
            // solution point is the smaller point
            double t0 = -b-disc;
            double t1 = -b+disc;
            if (t0 < t1) {return t0;}
            else {return t1;}
        }
    }

    public static class Ray {
        // a ray object takes in a origin and direction vector
        public Vector o, d;
        public Ray(Vector o, Vector d) {this.o = o;	this.d = d;}
    }

    public static Vector drawPixel(Sphere s, Vector pixCol, Ray ray, Sphere light) {
        double t = s.intersection(ray);
        Vector sln = ray.o.add(ray.d.mult(t));
        Vector l = light.c.sub(sln);
        Vector n = s.getNormal(sln);

        // dt is the amount of shading produced, determined
        // by Lambertian reflection equation
        double dt = l.normalize().dot(n.normalize());
        Vector white = new Vector(255, 255, 255);
        pixCol = (white.mult(dt).add(s.col)).mult(0.5);
        clamp(pixCol);
        return pixCol;
    }

    public static void main(String args[]) {
        long startTime = System.currentTimeMillis();

        // create a 500 x 500 image
        int h = 500;
        int w = 500;

        // define colors for reflection (white), the background (bg), and the color of the sphere (green)
        Vector bg = new Vector(16, 16, 16);
        Sphere light = new Sphere(new Vector(1000, -1000, 0), 100, new Vector(0, 0, 0));

        // create 2 spheres, one that will be rendered, and one that will act as global illumination
        Sphere s1 = new Sphere(new Vector(w * 0.5, h * 0.5, 50), 125, new Vector(160, 32, 240));
        Sphere s2 = new Sphere(new Vector(w * 0.75, h * 0.75, 50), 75, new Vector(255, 0, 0));
        Sphere s3 = new Sphere(new Vector(w * 0.25, h * 0.25, 10), 75, new Vector(0, 0, 255));

        File output = new File("java_out.ppm");
        try {
            BufferedWriter bw = new BufferedWriter(new FileWriter(output));
            // ppm header
            bw.write("P3\n"+w+" "+h+" 255\n");
            // variable for intersection
            Vector pixCol = bg;
            for (int y=0; y<h; ++y) {
                for (int x=0; x<w; ++x) {
                    pixCol = bg;
                    // a ray is sent from the current camera's pixel, out onto the scene
                    // if the ray intersects, determine the point of intersection
                    // and the direction of the reflected ray
                    Ray ray = new Ray(new Vector(x, y, 0), new Vector(0, 0, 1));
                    if (s3.intersects(ray)) {
                        pixCol = drawPixel(s3, pixCol, ray, light);
                    }
                    if (s2.intersects(ray)) {
                        pixCol = drawPixel(s2, pixCol, ray, light);
                    }
                    if (s1.intersects(ray)) {
                        pixCol = drawPixel(s1, pixCol, ray, light);
                    }
                    bw.write((int)pixCol.x+" ");
                    bw.write((int)pixCol.y+" ");
                    bw.write((int)pixCol.z+"\n");
                }
            }
            bw.flush();
            bw.close();
        }
        catch (IOException e) {e.printStackTrace();}
        long memory = Runtime.getRuntime().totalMemory() - Runtime.getRuntime().freeMemory();
        System.out.println("Memory Used: " + memory + " bytes");
        System.out.println("Memory Used: " + ((double) memory) / 1024.0f + " kb");
        System.out.println("Memory Used: " + (((double) memory) / 1024.0f) / 1024.0f + " mb");
        System.out.println("Succesfully wrote to output.ppm!");
        double estimatedTime = ((double)(System.currentTimeMillis() - startTime)) / 1000.0d;
        System.out.println("Elapsed: " + estimatedTime + " secs");
    }
    // clamp function sets boundaries for colors, assuring that RGB values don't go over 255
    public static void clamp(Vector color) {
        if (color.x > 255) {color.x = 255;}
        else if (color.x < 0) {color.x = 0;}
        if (color.y > 255) {color.y = 255;}
        else if (color.y < 0) {color.y = 0;}
        if (color.z > 255) {color.z = 255;}
        else if (color.z < 0) {color.z = 0;}
    }
}