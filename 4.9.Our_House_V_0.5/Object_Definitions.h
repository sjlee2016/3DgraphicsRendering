
// The object modelling tasks performed by this file are usually done 
// by reading a scene configuration file or through a help of graphics user interface!!!

#define BUFFER_OFFSET(offset) ((GLvoid *) (offset))


typedef struct _material {
	glm::vec4 emission, ambient, diffuse, specular;
	GLfloat exponent;
} Material;

float teapot_rotation_angle = 0.0f;
#define N_MAX_GEOM_COPIES 5
typedef struct _Object {
	char filename[512];

	GLenum front_face_mode; // clockwise or counter-clockwise
	int n_triangles;

	int n_fields; // 3 floats for vertex, 3 floats for normal, and 2 floats for texcoord
	GLfloat *vertices; // pointer to vertex array data
	GLfloat xmin, xmax, ymin, ymax, zmin, zmax; // bounding box <- compute this yourself

	GLuint VBO, VAO; // Handles to vertex buffer object and vertex array object

	int n_geom_instances;
	glm::mat4 ModelMatrix[N_MAX_GEOM_COPIES];
	Material_Parameters realMat;
	Material material[N_MAX_GEOM_COPIES];
} Object;
void set_material_object(Object * obj_ptr, int instance_ID);
#define N_GEOMETRY_OBJECTS 6
#define GEOM_OBJ_ID_CAR_BODY 0
#define GEOM_OBJ_ID_CAR_WHEEL 1
#define GEOM_OBJ_ID_CAR_NUT 2
#define GEOM_OBJ_ID_COW 3
#define GEOM_OBJ_ID_TEAPOT 4
#define GEOM_OBJ_ID_BOX 5
#define BUFFER_OFFSET(offset) ((GLvoid *) (offset))

#define LOC_VERTEX 0
#define LOC_NORMAL 1
#define LOC_TEXCOORD 2
GLuint geom_obj_VBO[N_GEOMETRY_OBJECTS];
GLuint geom_obj_VAO[N_GEOMETRY_OBJECTS];

int geom_obj_n_triangles[N_GEOMETRY_OBJECTS];
GLfloat *geom_obj_vertices[N_GEOMETRY_OBJECTS];

// codes for the 'general' triangular-mesh object
typedef enum _GEOM_OBJ_TYPE { GEOM_OBJ_TYPE_V = 0, GEOM_OBJ_TYPE_VN, GEOM_OBJ_TYPE_VNT } GEOM_OBJ_TYPE;
// GEOM_OBJ_TYPE_V: (x, y, z)
// GEOM_OBJ_TYPE_VN: (x, y, z, nx, ny, nz)
// GEOM_OBJ_TYPE_VNT: (x, y, z, nx, ny, nz, s, t)
int GEOM_OBJ_ELEMENTS_PER_VERTEX[3] = { 3, 6, 8 };

int read_geometry_file(GLfloat **object, char *filename, GEOM_OBJ_TYPE geom_obj_type) {
	int i, n_triangles;
	float *flt_ptr;
	FILE *fp;

	fprintf(stdout, "Reading geometry from the geometry file %s...\n", filename);
	fp = fopen(filename, "r");
	if (fp == NULL) {
		fprintf(stderr, "Cannot open the geometry file %s ...", filename);
		return -1;
	}

	fscanf(fp, "%d", &n_triangles);
	*object = (float *)malloc(3 * n_triangles*GEOM_OBJ_ELEMENTS_PER_VERTEX[geom_obj_type] * sizeof(float));
	if (*object == NULL) {
		fprintf(stderr, "Cannot allocate memory for the geometry file %s ...", filename);
		return -1;
	}

	flt_ptr = *object;
	for (i = 0; i < 3 * n_triangles * GEOM_OBJ_ELEMENTS_PER_VERTEX[geom_obj_type]; i++)
		fscanf(fp, "%f", flt_ptr++);
	fclose(fp);

	fprintf(stdout, "Read %d primitives successfully.\n\n", n_triangles);

	return n_triangles;
}

void prepare_geom_obj(int geom_obj_ID, char *filename, GEOM_OBJ_TYPE geom_obj_type) {
	int n_bytes_per_vertex;

	n_bytes_per_vertex = GEOM_OBJ_ELEMENTS_PER_VERTEX[geom_obj_type] * sizeof(float);
	geom_obj_n_triangles[geom_obj_ID] = read_geometry_file(&geom_obj_vertices[geom_obj_ID], filename, geom_obj_type);

	// Initialize vertex array object.
	glGenVertexArrays(1, &geom_obj_VAO[geom_obj_ID]);
	glBindVertexArray(geom_obj_VAO[geom_obj_ID]);
	glGenBuffers(1, &geom_obj_VBO[geom_obj_ID]);
	glBindBuffer(GL_ARRAY_BUFFER, geom_obj_VBO[geom_obj_ID]);
	glBufferData(GL_ARRAY_BUFFER, 3 * geom_obj_n_triangles[geom_obj_ID] * n_bytes_per_vertex,
		geom_obj_vertices[geom_obj_ID], GL_STATIC_DRAW);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	if (geom_obj_type >= GEOM_OBJ_TYPE_VN) {
		glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
	}
	if (geom_obj_type >= GEOM_OBJ_TYPE_VNT) {
		glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
		glEnableVertexAttribArray(2);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	free(geom_obj_vertices[geom_obj_ID]);
}

void draw_geom_obj(int geom_obj_ID) {
	glBindVertexArray(geom_obj_VAO[geom_obj_ID]);
	glDrawArrays(GL_TRIANGLES, 0, 3 * geom_obj_n_triangles[geom_obj_ID]);
	glBindVertexArray(0);
}

void free_geom_obj(int geom_obj_ID) {
	glDeleteVertexArrays(1, &geom_obj_VAO[geom_obj_ID]);
	glDeleteBuffers(1, &geom_obj_VBO[geom_obj_ID]);
}
#define N_MAX_STATIC_OBJECTS		10
Object static_objects[N_MAX_STATIC_OBJECTS]; // allocage memory dynamically every time it is needed rather than using a static array
int n_static_objects = 0;


#define OBJ_BUILDING		0
#define OBJ_TABLE			1
#define OBJ_LIGHT			2
#define OBJ_TEAPOT			3
#define OBJ_NEW_CHAIR		4
#define OBJ_FRAME			5
#define OBJ_SPIDER			6
#define OBJ_COW				7

int read_geometry(GLfloat **object, int bytes_per_primitive, char *filename) {
	int n_triangles;
	FILE *fp;

	// fprintf(stdout, "Reading geometry from the geometry file %s...\n", filename);
	fp = fopen(filename, "rb");
	if (fp == NULL) {
		fprintf(stderr, "Error: cannot open the object file %s ...\n", filename);
		exit(EXIT_FAILURE);
	}
	fread(&n_triangles, sizeof(int), 1, fp);
	*object = (float *)malloc(n_triangles*bytes_per_primitive);
	if (*object == NULL) {
		fprintf(stderr, "Error: cannot allocate memory for the geometry file %s ...\n", filename);
		exit(EXIT_FAILURE);
	}
	fread(*object, bytes_per_primitive, n_triangles, fp); // assume the data file has no faults.
														  // fprintf(stdout, "Read %d primitives successfully.\n\n", n_triangles);
	fclose(fp);

	return n_triangles;
}

void compute_AABB(Object *obj_ptr) {
	// Do it yourself.
}

void prepare_geom_of_static_object(Object *obj_ptr) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle;
	char filename[512];

	n_bytes_per_vertex = obj_ptr->n_fields * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	obj_ptr->n_triangles = read_geometry(&(obj_ptr->vertices), n_bytes_per_triangle, obj_ptr->filename);

	// Initialize vertex buffer object.
	glGenBuffers(1, &(obj_ptr->VBO));

	glBindBuffer(GL_ARRAY_BUFFER, obj_ptr->VBO);
	glBufferData(GL_ARRAY_BUFFER, obj_ptr->n_triangles*n_bytes_per_triangle, obj_ptr->vertices, GL_STATIC_DRAW);

	compute_AABB(obj_ptr);
	free(obj_ptr->vertices);

	// Initialize vertex array object.
	glGenVertexArrays(1, &(obj_ptr->VAO));
	glBindVertexArray(obj_ptr->VAO);

	glBindBuffer(GL_ARRAY_BUFFER, obj_ptr->VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
void copyArray(float * arr, glm::vec4 v)
{
	arr[0] = v.r;
	arr[1] = v.g;
	arr[2] = v.b;
	arr[3] = v.a;
	printf("in copy array\n");
	printf("%f %f %f %f \n", arr[0], arr[1], arr[2], arr[3]);
}

void copyMat(Material_Parameters * m, Material material)
{
	copyArray(m->ambient_color, material.ambient);
	printf("in copy Mat\n");
	printf("%f %f %f %f\n", m->ambient_color[0], m->ambient_color[1], m->ambient_color[2], m->ambient_color[3]);
	copyArray(m->emissive_color, material.emission);

	copyArray(m->specular_color, material.specular);

	copyArray(m->diffuse_color, material.diffuse);

	m->specular_exponent = material.exponent;

}
void define_static_objects(void) {
	// building
	strcpy(static_objects[OBJ_BUILDING].filename, "Data/Building1_vnt.geom");
	static_objects[OBJ_BUILDING].n_fields = 8;

	static_objects[OBJ_BUILDING].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_BUILDING]));

	static_objects[OBJ_BUILDING].n_geom_instances = 1;

	static_objects[OBJ_BUILDING].ModelMatrix[0] = glm::mat4(1.0f);


	static_objects[OBJ_BUILDING].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_BUILDING].material[0].ambient = glm::vec4(0.135f, 0.2225f, 0.1575f, 1.0f);
	static_objects[OBJ_BUILDING].material[0].diffuse = glm::vec4(0.54f, 0.89f, 0.63f, 1.0f);
	static_objects[OBJ_BUILDING].material[0].specular = glm::vec4(0.316228f, 0.316228f, 0.316228f, 1.0f);
	static_objects[OBJ_BUILDING].material[0].exponent = 128.0f*0.1f;
	copyMat(&(static_objects[OBJ_BUILDING].realMat), static_objects[OBJ_BUILDING].material[0]);
	// table
	strcpy(static_objects[OBJ_TABLE].filename, "Data/Table_vn.geom");
	static_objects[OBJ_TABLE].n_fields = 6;

	static_objects[OBJ_TABLE].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_TABLE]));

	static_objects[OBJ_TABLE].n_geom_instances = 2;

	static_objects[OBJ_TABLE].ModelMatrix[0] = glm::translate(glm::mat4(1.0f), glm::vec3(157.0f, 76.5f, 0.0f));
	static_objects[OBJ_TABLE].ModelMatrix[0] = glm::scale(static_objects[OBJ_TABLE].ModelMatrix[0],
		glm::vec3(0.5f, 0.5f, 0.5f));



	static_objects[OBJ_TABLE].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_TABLE].material[0].ambient = glm::vec4(0.1f, 0.3f, 0.1f, 1.0f);
	static_objects[OBJ_TABLE].material[0].diffuse = glm::vec4(0.4f, 0.6f, 0.3f, 1.0f);
	static_objects[OBJ_TABLE].material[0].specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	static_objects[OBJ_TABLE].material[0].exponent = 15.0f;

	static_objects[OBJ_TABLE].ModelMatrix[1] = glm::translate(glm::mat4(1.0f), glm::vec3(198.0f, 120.0f, 0.0f));
	static_objects[OBJ_TABLE].ModelMatrix[1] = glm::scale(static_objects[OBJ_TABLE].ModelMatrix[1],
		glm::vec3(0.8f, 0.6f, 0.6f));

	static_objects[OBJ_TABLE].material[1].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_TABLE].material[1].ambient = glm::vec4(0.05f, 0.05f, 0.05f, 1.0f);
	static_objects[OBJ_TABLE].material[1].diffuse = glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);
	static_objects[OBJ_TABLE].material[1].specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	static_objects[OBJ_TABLE].material[1].exponent = 128.0f*0.078125f;
	copyMat(&(static_objects[OBJ_TABLE].realMat), static_objects[OBJ_TABLE].material[0]);

	// Light
	strcpy(static_objects[OBJ_LIGHT].filename, "Data/Light_vn.geom");
	static_objects[OBJ_LIGHT].n_fields = 6;

	static_objects[OBJ_LIGHT].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(static_objects + OBJ_LIGHT);

	static_objects[OBJ_LIGHT].n_geom_instances = 5;

	static_objects[OBJ_LIGHT].ModelMatrix[0] = glm::translate(glm::mat4(1.0f), glm::vec3(120.0f, 100.0f, 49.0f));
	static_objects[OBJ_LIGHT].ModelMatrix[0] = glm::rotate(static_objects[OBJ_LIGHT].ModelMatrix[0],
		90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));

	static_objects[OBJ_LIGHT].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_LIGHT].material[0].ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
	static_objects[OBJ_LIGHT].material[0].diffuse = glm::vec4(0.75164f, 0.60648f, 0.22648f, 1.0f);
	static_objects[OBJ_LIGHT].material[0].specular = glm::vec4(0.628281f, 0.555802f, 0.366065f, 1.0f);
	static_objects[OBJ_LIGHT].material[0].exponent = 128.0f*0.4f;

	static_objects[OBJ_LIGHT].ModelMatrix[1] = glm::translate(glm::mat4(1.0f), glm::vec3(80.0f, 47.5f, 49.0f));
	static_objects[OBJ_LIGHT].ModelMatrix[1] = glm::rotate(static_objects[OBJ_LIGHT].ModelMatrix[1],
		90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));

	static_objects[OBJ_LIGHT].material[1].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_LIGHT].material[1].ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
	static_objects[OBJ_LIGHT].material[1].diffuse = glm::vec4(0.75164f, 0.60648f, 0.22648f, 1.0f);
	static_objects[OBJ_LIGHT].material[1].specular = glm::vec4(0.628281f, 0.555802f, 0.366065f, 1.0f);
	static_objects[OBJ_LIGHT].material[1].exponent = 128.0f*0.4f;

	static_objects[OBJ_LIGHT].ModelMatrix[2] = glm::translate(glm::mat4(1.0f), glm::vec3(40.0f, 130.0f, 49.0f));
	static_objects[OBJ_LIGHT].ModelMatrix[2] = glm::rotate(static_objects[OBJ_LIGHT].ModelMatrix[2],
		90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));

	static_objects[OBJ_LIGHT].material[2].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_LIGHT].material[2].ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
	static_objects[OBJ_LIGHT].material[2].diffuse = glm::vec4(0.75164f, 0.60648f, 0.22648f, 1.0f);
	static_objects[OBJ_LIGHT].material[2].specular = glm::vec4(0.628281f, 0.555802f, 0.366065f, 1.0f);
	static_objects[OBJ_LIGHT].material[2].exponent = 128.0f*0.4f;

	static_objects[OBJ_LIGHT].ModelMatrix[3] = glm::translate(glm::mat4(1.0f), glm::vec3(190.0f, 60.0f, 49.0f));
	static_objects[OBJ_LIGHT].ModelMatrix[3] = glm::rotate(static_objects[OBJ_LIGHT].ModelMatrix[3],
		90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));

	static_objects[OBJ_LIGHT].material[3].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_LIGHT].material[3].ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
	static_objects[OBJ_LIGHT].material[3].diffuse = glm::vec4(0.75164f, 0.60648f, 0.22648f, 1.0f);
	static_objects[OBJ_LIGHT].material[3].specular = glm::vec4(0.628281f, 0.555802f, 0.366065f, 1.0f);
	static_objects[OBJ_LIGHT].material[3].exponent = 128.0f*0.4f;

	static_objects[OBJ_LIGHT].ModelMatrix[4] = glm::translate(glm::mat4(1.0f), glm::vec3(210.0f, 112.5f, 49.0));
	static_objects[OBJ_LIGHT].ModelMatrix[4] = glm::rotate(static_objects[OBJ_LIGHT].ModelMatrix[4],
		90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));

	static_objects[OBJ_LIGHT].material[4].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_LIGHT].material[4].ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
	static_objects[OBJ_LIGHT].material[4].diffuse = glm::vec4(0.75164f, 0.60648f, 0.22648f, 1.0f);
	static_objects[OBJ_LIGHT].material[4].specular = glm::vec4(0.628281f, 0.555802f, 0.366065f, 1.0f);
	static_objects[OBJ_LIGHT].material[4].exponent = 128.0f*0.4f;
	copyMat(&(static_objects[OBJ_LIGHT].realMat), static_objects[OBJ_LIGHT].material[0]);

	// teapot
	strcpy(static_objects[OBJ_TEAPOT].filename, "Data/Teapotn_vn.geom");
	static_objects[OBJ_TEAPOT].n_fields = 6;

	static_objects[OBJ_TEAPOT].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_TEAPOT]));

	static_objects[OBJ_TEAPOT].n_geom_instances = 1;
	static_objects[OBJ_TEAPOT].ModelMatrix[0] = glm::translate(glm::mat4(1.0f), glm::vec3(193.0f, 120.0f, 11.0f));
	static_objects[OBJ_TEAPOT].ModelMatrix[0] = glm::scale(static_objects[OBJ_TEAPOT].ModelMatrix[0],
		glm::vec3(2.0f, 2.0f, 2.0f));

	static_objects[OBJ_TEAPOT].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_TEAPOT].material[0].ambient = glm::vec4(0.1745f, 0.01175f, 0.01175f, 1.0f);
	static_objects[OBJ_TEAPOT].material[0].diffuse = glm::vec4(0.61424f, 0.04136f, 0.04136f, 1.0f);
	static_objects[OBJ_TEAPOT].material[0].specular = glm::vec4(0.727811f, 0.626959f, 0.626959f, 1.0f);
	static_objects[OBJ_TEAPOT].material[0].exponent = 128.0f*0.6;


	static_objects[OBJ_TEAPOT].ModelMatrix[1] = glm::translate(glm::mat4(1.0f), glm::vec3(193.0f, 120.0f, 15.0f));
	static_objects[OBJ_TEAPOT].ModelMatrix[1] = glm::scale(static_objects[OBJ_TEAPOT].ModelMatrix[1],
		glm::vec3(2.0f, 2.0f, 2.0f));

	static_objects[OBJ_TEAPOT].material[1].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_TEAPOT].material[1].ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
	static_objects[OBJ_TEAPOT].material[1].diffuse = glm::vec4(0.75164f, 0.60648f, 0.22648f, 1.0f);
	static_objects[OBJ_TEAPOT].material[1].specular = glm::vec4(0.628281f, 0.555802f, 0.366065f, 1.0f);
	static_objects[OBJ_TEAPOT].material[1].exponent = 128.0f*0.6;

	static_objects[OBJ_TEAPOT].ModelMatrix[2] = glm::rotate(static_objects[OBJ_TEAPOT].ModelMatrix[2], 90.0f * TO_RADIAN,
		glm::vec3(0.0f, 1.0f, 0.0f));

	static_objects[OBJ_TEAPOT].ModelMatrix[2] = glm::translate(glm::mat4(1.0f), glm::vec3(193.0f, 120.0f, 20.0f));

	static_objects[OBJ_TEAPOT].ModelMatrix[2] = glm::scale(static_objects[OBJ_TEAPOT].ModelMatrix[2],
		glm::vec3(2.0f, 2.0f, 2.0f));

	static_objects[OBJ_TEAPOT].material[2].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_TEAPOT].material[2].ambient = glm::vec4(0.19125f, 0.0735f, 0.0225f, 1.0f);
	static_objects[OBJ_TEAPOT].material[2].diffuse = glm::vec4(0.7038f, 0.27048f, 0.0828f, 1.0f);
	static_objects[OBJ_TEAPOT].material[2].specular = glm::vec4(0.256777f, 0.137622f, 0.086014f, 1.0f);
	static_objects[OBJ_TEAPOT].material[2].exponent = 128.0f*0.1f;
	copyMat(&(static_objects[OBJ_TEAPOT].realMat), static_objects[OBJ_TEAPOT].material[2]);



	strcpy(static_objects[OBJ_SPIDER].filename, "Data/spider/spider_vnt_00.geom");
	static_objects[OBJ_SPIDER].n_fields = 8;

	static_objects[OBJ_SPIDER].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_SPIDER]));

	static_objects[OBJ_SPIDER].n_geom_instances = 1;
	static_objects[OBJ_SPIDER].ModelMatrix[0] = glm::translate(glm::mat4(1.0f), glm::vec3(193.0f, 120.0f, 5.0f));
	static_objects[OBJ_SPIDER].ModelMatrix[0] = glm::scale(static_objects[OBJ_SPIDER].ModelMatrix[0],
		glm::vec3(2.0f, 2.0f, 2.0f));


	static_objects[OBJ_SPIDER].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_SPIDER].material[0].ambient = glm::vec4(0.25f, 0.25f, 0.25f, 1.0f);
	static_objects[OBJ_SPIDER].material[0].diffuse = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
	static_objects[OBJ_SPIDER].material[0].specular = glm::vec4(0.774597f, 0.774597f, 0.774597f, 1.0f);
	static_objects[OBJ_SPIDER].material[0].exponent = 128.0f*0.6f;
	copyMat(&(static_objects[OBJ_SPIDER].realMat), static_objects[OBJ_SPIDER].material[0]);

	// new_chair
	strcpy(static_objects[OBJ_NEW_CHAIR].filename, "Data/new_chair_vnt.geom");
	static_objects[OBJ_NEW_CHAIR].n_fields = 8;

	static_objects[OBJ_NEW_CHAIR].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_NEW_CHAIR]));

	static_objects[OBJ_NEW_CHAIR].n_geom_instances = 1;

	static_objects[OBJ_NEW_CHAIR].ModelMatrix[0] = glm::translate(glm::mat4(1.0f), glm::vec3(200.0f, 104.0f, 0.0f));
	static_objects[OBJ_NEW_CHAIR].ModelMatrix[0] = glm::scale(static_objects[OBJ_NEW_CHAIR].ModelMatrix[0],
		glm::vec3(0.8f, 0.8f, 0.8f));
	static_objects[OBJ_NEW_CHAIR].ModelMatrix[0] = glm::rotate(static_objects[OBJ_NEW_CHAIR].ModelMatrix[0],
		180.0f*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	static_objects[OBJ_NEW_CHAIR].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_NEW_CHAIR].material[0].ambient = glm::vec4(0.05f, 0.05f, 0.0f, 1.0f);
	static_objects[OBJ_NEW_CHAIR].material[0].diffuse = glm::vec4(0.5f, 0.5f, 0.4f, 1.0f);
	static_objects[OBJ_NEW_CHAIR].material[0].specular = glm::vec4(0.7f, 0.7f, 0.04f, 1.0f);
	static_objects[OBJ_NEW_CHAIR].material[0].exponent = 128.0f*0.078125f;


	static_objects[OBJ_NEW_CHAIR].ModelMatrix[1] = glm::translate(glm::mat4(1.0f), glm::vec3(215.0f, 110.0f, 0.0f));
	static_objects[OBJ_NEW_CHAIR].ModelMatrix[1] = glm::scale(static_objects[OBJ_NEW_CHAIR].ModelMatrix[1],
		glm::vec3(0.8f, 0.8f, 0.8f));
	static_objects[OBJ_NEW_CHAIR].ModelMatrix[1] = glm::rotate(static_objects[OBJ_NEW_CHAIR].ModelMatrix[1],
		-90.0f*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	static_objects[OBJ_NEW_CHAIR].material[1].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_NEW_CHAIR].material[1].ambient = glm::vec4(0.05f, 0.05f, 0.0f, 1.0f);
	static_objects[OBJ_NEW_CHAIR].material[1].diffuse = glm::vec4(0.5f, 0.5f, 0.4f, 1.0f);
	static_objects[OBJ_NEW_CHAIR].material[1].specular = glm::vec4(0.7f, 0.7f, 0.04f, 1.0f);
	static_objects[OBJ_NEW_CHAIR].material[1].exponent = 128.0f*0.078125f;
	copyMat(&(static_objects[OBJ_NEW_CHAIR].realMat), static_objects[OBJ_NEW_CHAIR].material[0]);

	// frame
	strcpy(static_objects[OBJ_FRAME].filename, "Data/Frame_vn.geom");
	static_objects[OBJ_FRAME].n_fields = 6;

	static_objects[OBJ_FRAME].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_FRAME]));

	static_objects[OBJ_FRAME].n_geom_instances = 1;

	static_objects[OBJ_FRAME].ModelMatrix[0] = glm::translate(glm::mat4(1.0f), glm::vec3(188.0f, 116.0f, 30.0f));
	static_objects[OBJ_FRAME].ModelMatrix[0] = glm::scale(static_objects[OBJ_FRAME].ModelMatrix[0],
		glm::vec3(0.6f, 0.6f, 0.6f));
	static_objects[OBJ_FRAME].ModelMatrix[0] = glm::rotate(static_objects[OBJ_FRAME].ModelMatrix[0],
		90.0f*TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));

	static_objects[OBJ_FRAME].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_FRAME].material[0].ambient = glm::vec4(0.19125f, 0.0735f, 0.0225f, 1.0f);
	static_objects[OBJ_FRAME].material[0].diffuse = glm::vec4(0.7038f, 0.27048f, 0.0828f, 1.0f);
	static_objects[OBJ_FRAME].material[0].specular = glm::vec4(0.256777f, 0.137622f, 0.086014f, 1.0f);
	static_objects[OBJ_FRAME].material[0].exponent = 128.0f*0.1f;
	copyMat(&(static_objects[OBJ_FRAME].realMat), static_objects[OBJ_FRAME].material[1]);

	printf("%f %f \n", static_objects[OBJ_FRAME].realMat.diffuse_color[0], static_objects[OBJ_FRAME].realMat.diffuse_color[1]);
	// new_picture
	/*strcpy(static_objects[OBJ_NEW_PICTURE].filename, "Data/spider/spider_vnt_00.geom");
	static_objects[OBJ_NEW_PICTURE].n_fields = 8;

	static_objects[OBJ_NEW_PICTURE].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_NEW_PICTURE]));

	static_objects[OBJ_NEW_PICTURE].n_geom_instances = 1;

	static_objects[OBJ_NEW_PICTURE].ModelMatrix[0] = glm::translate(glm::mat4(1.0f), glm::vec3(189.5f, 116.0f, 30.0f));
	static_objects[OBJ_NEW_PICTURE].ModelMatrix[0] = glm::scale(static_objects[OBJ_NEW_PICTURE].ModelMatrix[0],
	glm::vec3(13.5f*0.6f, 13.5f*0.6f, 13.5f*0.6f));
	static_objects[OBJ_NEW_PICTURE].ModelMatrix[0] = glm::rotate(static_objects[OBJ_NEW_PICTURE].ModelMatrix[0],
	90.0f*TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));

	static_objects[OBJ_NEW_PICTURE].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_NEW_PICTURE].material[0].ambient = glm::vec4(0.25f, 0.25f, 0.25f, 1.0f);
	static_objects[OBJ_NEW_PICTURE].material[0].diffuse = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
	static_objects[OBJ_NEW_PICTURE].material[0].specular = glm::vec4(0.774597f, 0.774597f, 0.774597f, 1.0f);
	static_objects[OBJ_NEW_PICTURE].material[0].exponent = 128.0f*0.6f;
	*/
	// new_picture
	strcpy(static_objects[OBJ_COW].filename, "Data/cow_vn.geom");
	static_objects[OBJ_COW].n_fields = 6;

	static_objects[OBJ_COW].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_COW]));

	static_objects[OBJ_COW].n_geom_instances = 1;

	static_objects[OBJ_COW].ModelMatrix[0] = glm::translate(glm::mat4(1.0f), glm::vec3(200.0f, 150.0f, 9.5f));
	static_objects[OBJ_COW].ModelMatrix[0] = glm::scale(static_objects[OBJ_COW].ModelMatrix[0],
		glm::vec3(30.0f, 30.0f, 30.0f));
	//static_objects[OBJ_COW].ModelMatrix[0] = glm::rotate(static_objects[OBJ_COW].ModelMatrix[0],
	//	90.0f*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	static_objects[OBJ_COW].ModelMatrix[0] = glm::rotate(static_objects[OBJ_COW].ModelMatrix[0],
		90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));


	//static_objects[OBJ_COW].ModelMatrix[1] = glm::translate(static_objects[OBJ_COW].ModelMatrix[1], glm::vec3(0.0f, 100.0f, 0.0f));
	static_objects[OBJ_COW].ModelMatrix[1] = glm::translate(glm::mat4(1.0f), glm::vec3(210.0f, 160.0f, 9.5f));

	static_objects[OBJ_COW].ModelMatrix[1] = glm::scale(static_objects[OBJ_COW].ModelMatrix[1],
		glm::vec3(10.0f, 10.0f, 10.0f));
	//static_objects[OBJ_COW].ModelMatrix[1] = glm::rotate(static_objects[OBJ_COW].ModelMatrix[1],
	//90.0f*TO_RADIAN, glm::vec3(0.5f, 0.5f, 0.0f));
	static_objects[OBJ_COW].ModelMatrix[1] = glm::rotate(static_objects[OBJ_COW].ModelMatrix[1],
		-0.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	static_objects[OBJ_COW].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_COW].material[0].ambient = glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f);
	static_objects[OBJ_COW].material[0].diffuse = glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f);
	static_objects[OBJ_COW].material[0].specular = glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f);
	static_objects[OBJ_COW].material[0].exponent = 0.21794872f*0.6f;

	/*
	
	static_objects[OBJ_COW].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_COW].material[0].ambient = glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f);
	static_objects[OBJ_COW].material[0].diffuse = glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f);
	static_objects[OBJ_COW].material[0].specular = glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f);
	static_objects[OBJ_COW].material[0].exponent = 0.21794872f*0.6f;

	*/
	copyMat(&(static_objects[OBJ_COW].realMat), static_objects[OBJ_COW].material[0]);
	

	static_objects[OBJ_COW].material[1].emission = glm::vec4(0.1f, 0.1f, 0.0f, 1.0f);
	static_objects[OBJ_COW].material[1].ambient = glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f);
	static_objects[OBJ_COW].material[1].diffuse = glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f);
	static_objects[OBJ_COW].material[1].specular = glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f);
	static_objects[OBJ_COW].material[1].exponent = 0.21794872f*0.6f;

	n_static_objects = 8;
}

void draw_static_object(Object *obj_ptr, int instance_ID, int cam_index) {
	glFrontFace(obj_ptr->front_face_mode);
	//glFrontFace(GL_CCW);

	ModelViewMatrix = ViewMatrix[cam_index] * obj_ptr->ModelMatrix[instance_ID];
	ModelViewProjectionMatrix = ProjectionMatrix[cam_index] * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1.0, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

	//set_material_object(obj_ptr, instance_ID);
	glBindVertexArray(obj_ptr->VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * obj_ptr->n_triangles);
	glBindVertexArray(0);
}



GLuint VBO_axes, VAO_axes;
GLfloat vertices_axes[6][3] = {
	{ 0.0f, 0.0f, 0.0f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f },
{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 1.0f }
};
GLfloat axes_color[3][3] = { { 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } };

void define_axes(void) {  
	glGenBuffers(1, &VBO_axes);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_axes);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_axes), &vertices_axes[0][0], GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_axes);
	glBindVertexArray(VAO_axes);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_axes);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

#define WC_AXIS_LENGTH		60.0f
void draw_axes(int cam_index) {
	ModelViewMatrix = glm::scale(ViewMatrix[cam_index], glm::vec3(WC_AXIS_LENGTH, WC_AXIS_LENGTH, WC_AXIS_LENGTH));
	ModelViewProjectionMatrix = ProjectionMatrix[cam_index] * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);

	glBindVertexArray(VAO_axes);
	glUniform3fv(loc_primitive_color, 1, axes_color[0]);
	glDrawArrays(GL_LINES, 0, 2);
	glUniform3fv(loc_primitive_color, 1, axes_color[1]);
	glDrawArrays(GL_LINES, 2, 2);
	glUniform3fv(loc_primitive_color, 1, axes_color[2]);
	glDrawArrays(GL_LINES, 4, 2);
	glBindVertexArray(0);
}


GLuint rectangle_VBO, rectangle_VAO;
GLfloat rectangle_vertices[12][3] = {  // vertices enumerated counterclockwise
	{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 1.0f },
{ 1.0f, 1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 1.0f },
{ 1.0f, 1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f }
};

Material_Parameters material_floor;

void set_material_object(Object * obj_ptr, int instance_ID)
{

	glUniform4f(loc_material.ambient_color, obj_ptr->material[instance_ID].ambient.x, obj_ptr->material[instance_ID].ambient.y, obj_ptr->material[instance_ID].ambient.z, obj_ptr->material[instance_ID].ambient.w);
	glUniform4f(loc_material.emissive_color, obj_ptr->material[instance_ID].emission.x, obj_ptr->material[instance_ID].emission.y, obj_ptr->material[instance_ID].emission.z, obj_ptr->material[instance_ID].emission.w);
	glUniform4f(loc_material.specular_color, obj_ptr->material[instance_ID].specular.x, obj_ptr->material[instance_ID].specular.y, obj_ptr->material[instance_ID].specular.z, obj_ptr->material[instance_ID].specular.w);
	glUniform4f(loc_material.diffuse_color, obj_ptr->material[instance_ID].diffuse.x, obj_ptr->material[instance_ID].diffuse.y, obj_ptr->material[instance_ID].diffuse.z, obj_ptr->material[instance_ID].diffuse.w);
	glUniform1f(loc_material.specular_exponent, obj_ptr->material[instance_ID].exponent);

}

void prepare_floor(void) { // Draw coordinate axes.
						   // Initialize vertex buffer object.
	glGenBuffers(1, &rectangle_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, rectangle_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectangle_vertices), &rectangle_vertices[0][0], GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &rectangle_VAO);
	glBindVertexArray(rectangle_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, rectangle_VBO);
	glVertexAttribPointer(LOC_POSITION, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	/* 

	static_objects[OBJ_BUILDING].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_BUILDING].material[0].ambient = glm::vec4(0.135f, 0.2225f, 0.1575f, 1.0f);
	static_objects[OBJ_BUILDING].material[0].diffuse = glm::vec4(0.54f, 0.89f, 0.63f, 1.0f);
	static_objects[OBJ_BUILDING].material[0].specular = glm::vec4(0.316228f, 0.316228f, 0.316228f, 1.0f);
	static_objects[OBJ_BUILDING].material[0].exponent = 128.0f*0.1f;
	*/
	material_floor.ambient_color[0] = 0.135f;
	material_floor.ambient_color[1] = 0.2225f;
	material_floor.ambient_color[2] = 0.1575f;
	material_floor.ambient_color[3] = 1.0f;

	material_floor.diffuse_color[0] = 0.54f;
	material_floor.diffuse_color[1] = 0.89f;
	material_floor.diffuse_color[2] = 0.63f;
	material_floor.diffuse_color[3] = 1.0f;

	material_floor.specular_color[0] = 0.316228f;
	material_floor.specular_color[1] = 0.316228;
	material_floor.specular_color[2] = 0.316228;
	material_floor.specular_color[3] = 1.0f;

	material_floor.specular_exponent = 2.5f;

	material_floor.emissive_color[0] = 0.0f;
	material_floor.emissive_color[1] = 0.0f;
	material_floor.emissive_color[2] = 0.0f;
	material_floor.emissive_color[3] = 1.0f;
}

void set_material_floor(void) {
	// assume ShaderProgram_PS is used
	glUniform4fv(loc_material.ambient_color, 1, material_floor.ambient_color);
	glUniform4fv(loc_material.diffuse_color, 1, material_floor.diffuse_color);
	glUniform4fv(loc_material.specular_color, 1, material_floor.specular_color);
	glUniform1f(loc_material.specular_exponent, material_floor.specular_exponent);
	glUniform4fv(loc_material.emissive_color, 1, material_floor.emissive_color);
}

void draw_floor(int cam_index) {
	glFrontFace(GL_CCW);
	ModelViewMatrix = glm::translate(ViewMatrix[cam_index], glm::vec3(10.0f, 10.0f, 0.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(200.0f, 150.0f, 100.0f));
	//ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewProjectionMatrix = ProjectionMatrix[cam_index] * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));

	//ModelViewMatrix = glm::scale(ViewMatrix[cam_index], glm::vec3(0.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

	ModelViewProjectionMatrix = ProjectionMatrix[cam_index] * ModelViewMatrix;

	glBindVertexArray(rectangle_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

GLuint VBO_view, VAO_view;
GLfloat view_line[16][3] = {
	{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f },
{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f } , { 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f },
{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f },
{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f } ,{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f }
};
GLfloat view_color[3] = { 1.0f, 0.0f, 0.0f };

void define_view_volume(void) { 
	float x = camera[0].pos.x;
	float y = camera[0].pos.y;
	float z = camera[0].pos.z;
	float fov_y = camera[0].fov_y * TO_RADIAN;
	float far_clip = camera[0].far_clip;
	float d = far_clip + z;
	float height = (tan((fov_y)/ 2) * far_clip);
	float width = (height*camera[0].aspect_ratio)/2;
	for (int i = 0; i <= 6; i += 2)
	{
		view_line[i][0] = x;
		view_line[i][1] = y;
		view_line[i][2] = z;
	}

	view_line[1][0] = x - width;
	view_line[1][1] = y - height;
	view_line[1][2] = d;


	view_line[3][0] = x + width;
	view_line[3][1] = y - height;
	view_line[3][2] = d;



	view_line[5][0] = x - width;
	view_line[5][1] = y + height;
	view_line[5][2] = d;

	////////////////////////////////////////
	view_line[7][0] = x + width;
	view_line[7][1] = y + height;
	view_line[7][2] = d;



	view_line[8][0] = x + width;
	view_line[8][1] = y - height;
	view_line[8][2] = d;

	/////////////////////////
	view_line[9][0] = x + width;
	view_line[9][1] = y + height;
	view_line[9][2] = d;



	view_line[10][0] = x - width;
	view_line[10][1] = y + height;
	view_line[10][2] = d;
	/////////////////////////

	view_line[11][0] = x + width;
	view_line[11][1] = y + height;
	view_line[11][2] = d;


	view_line[12][0] = x - width;
	view_line[12][1] = y - height;
	view_line[12][2] = d;
	////////////////////////

	view_line[13][0] = x + width;
	view_line[13][1] = y - height;
	view_line[13][2] = d;


	view_line[14][0] = x - width;
	view_line[14][1] = y - height;
	view_line[14][2] = d;

	////////////////////////////


	view_line[15][0] = x - width;
	view_line[15][1] = y + height;
	view_line[15][2] = d;



	glGenBuffers(1, &VBO_view);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_view);
	glBufferData(GL_ARRAY_BUFFER, sizeof(view_line), &view_line[0][0], GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_view);
	glBindVertexArray(VAO_view);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_view);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

#define WC_VIEW_LENGTH		60.0f
void draw_view(int cam_index) {
	define_view_volume();

	ModelViewMatrix = glm::scale(ViewMatrix[cam_index], glm::vec3(1.0f, 1.0f, 1.0f));

	//ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewProjectionMatrix = ProjectionMatrix[cam_index] * ModelViewMatrix;

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);

	glBindVertexArray(VAO_view);
	glUniform3fv(loc_primitive_color, 1, view_color);
	glDrawArrays(GL_LINES, 0, 2);
	glUniform3fv(loc_primitive_color, 1, view_color);
	glDrawArrays(GL_LINES, 2, 2);
	glUniform3fv(loc_primitive_color, 1, view_color);
	glDrawArrays(GL_LINES, 4, 2);

	glUniform3fv(loc_primitive_color, 1, view_color);
	glDrawArrays(GL_LINES, 6, 2);
	glUniform3fv(loc_primitive_color, 1, view_color);
	glDrawArrays(GL_LINES, 8, 2);
	glUniform3fv(loc_primitive_color, 1, view_color);
	glDrawArrays(GL_LINES, 10, 2);

	glUniform3fv(loc_primitive_color, 1, view_color);
	glDrawArrays(GL_LINES, 12, 2);
	glUniform3fv(loc_primitive_color, 1, view_color);
	glDrawArrays(GL_LINES, 14, 2);
	glUniform3fv(loc_primitive_color, 1, view_color);
	glDrawArrays(GL_LINES, 16, 2);

	glUniform3fv(loc_primitive_color, 1, view_color);
	glDrawArrays(GL_LINES, 18, 2);
	glUniform3fv(loc_primitive_color, 1, view_color);
	glDrawArrays(GL_LINES, 20, 2);
	glUniform3fv(loc_primitive_color, 1, view_color);
	glDrawArrays(GL_LINES, 22, 2);
	glUniform3fv(loc_primitive_color, 1, view_color);
	glDrawArrays(GL_LINES, 24, 2);

	glUniform3fv(loc_primitive_color, 1, view_color);
	glDrawArrays(GL_LINES, 26, 2);
	glUniform3fv(loc_primitive_color, 1, view_color);
	glDrawArrays(GL_LINES, 28, 2);
	glUniform3fv(loc_primitive_color, 1, view_color);
	glDrawArrays(GL_LINES, 30, 2);

	glUniform3fv(loc_primitive_color, 1, view_color);
	glDrawArrays(GL_LINES, 32, 2);
	glBindVertexArray(0);
}
Object teapot;
Object car[3];
#define N_TIGER_FRAMES 12
GLuint tiger_VBO, tiger_VAO;
int tiger_n_triangles[N_TIGER_FRAMES];
int tiger_vertex_offset[N_TIGER_FRAMES];
GLfloat *tiger_vertices[N_TIGER_FRAMES];
Material_Parameters material_tiger; 

Object tiger[N_TIGER_FRAMES];
struct {
	int cur_frame = 0;
	float rotation_angle = 0.0f;
} tiger_data;
void define_animated_tiger(void) {


	int i, n_bytes_per_vertex, n_bytes_per_triangle, tiger_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	for (i = 0; i < N_TIGER_FRAMES; i++) {
		sprintf(filename, "Data/Tiger_%d%d_triangles_vnt.geom", i / 10, i % 10);
		tiger_n_triangles[i] = read_geometry(&tiger_vertices[i], n_bytes_per_triangle, filename);
		// assume all geometry files are effective
		tiger[i].ModelMatrix[0] = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));

		tiger_n_total_triangles += tiger_n_triangles[i];

		if (i == 0)
			tiger_vertex_offset[i] = 0;
		else
			tiger_vertex_offset[i] = tiger_vertex_offset[i - 1] + 3 * tiger_n_triangles[i - 1];
	}

	// initialize vertex buffer object
	glGenBuffers(1, &tiger_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, tiger_VBO);
	glBufferData(GL_ARRAY_BUFFER, tiger_n_total_triangles*n_bytes_per_triangle, NULL, GL_STATIC_DRAW);

	for (i = 0; i < N_TIGER_FRAMES; i++)
		glBufferSubData(GL_ARRAY_BUFFER, tiger_vertex_offset[i] * n_bytes_per_vertex,
			tiger_n_triangles[i] * n_bytes_per_triangle, tiger_vertices[i]);

	// as the geometry data exists now in graphics memory, ...
	for (i = 0; i < N_TIGER_FRAMES; i++)
		free(tiger_vertices[i]);

	// initialize vertex array object
	glGenVertexArrays(1, &tiger_VAO);
	glBindVertexArray(tiger_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, tiger_VBO);
	glVertexAttribPointer(LOC_POSITION, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	material_tiger.ambient_color[0] = 0.24725f;
	material_tiger.ambient_color[1] = 0.1995f;
	material_tiger.ambient_color[2] = 0.0745f;
	material_tiger.ambient_color[3] = 1.0f;

	material_tiger.diffuse_color[0] = 0.75164f;
	material_tiger.diffuse_color[1] = 0.60648f;
	material_tiger.diffuse_color[2] = 0.22648f;
	material_tiger.diffuse_color[3] = 1.0f;

	material_tiger.specular_color[0] = 0.628281f;
	material_tiger.specular_color[1] = 0.555802f;
	material_tiger.specular_color[2] = 0.366065f;
	material_tiger.specular_color[3] = 1.0f;

	material_tiger.specular_exponent = 51.2f;

	material_tiger.emissive_color[0] = 0.1f;
	material_tiger.emissive_color[1] = 0.1f;
	material_tiger.emissive_color[2] = 0.0f;
	material_tiger.emissive_color[3] = 1.0f;
}
void set_material_tiger(void)
{
	glUniform4fv(loc_material.ambient_color, 1, material_tiger.ambient_color);
	glUniform4fv(loc_material.diffuse_color, 1, material_tiger.diffuse_color);
	glUniform4fv(loc_material.specular_color, 1, material_tiger.specular_color);
	glUniform1f(loc_material.specular_exponent, material_tiger.specular_exponent);
	glUniform4fv(loc_material.emissive_color, 1, material_tiger.emissive_color);
}
void define_animated_car(void)
{
	char carFile[3][40];
	strcpy(carFile[0],"Data/car_body_triangles_v.txt");
	strcpy(carFile[2], "Data/car_wheel_triangles_v.txt");
	strcpy(carFile[1] ,"Data/car_nut_triangles_v.txt");
	prepare_geom_obj(GEOM_OBJ_ID_CAR_BODY, carFile[0], GEOM_OBJ_TYPE_V);
	prepare_geom_obj(GEOM_OBJ_ID_CAR_WHEEL, carFile[2], GEOM_OBJ_TYPE_V);
	prepare_geom_obj(GEOM_OBJ_ID_CAR_NUT, carFile[1], GEOM_OBJ_TYPE_V);
}

void define_animated_teapot(void) {

	strcpy(teapot.filename, "Data/Teapotn_vn.geom");
	teapot.n_fields = 6;

	teapot.front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(teapot));

	teapot.n_geom_instances = 1;
	teapot.ModelMatrix[0] = glm::scale(teapot.ModelMatrix[0],
		glm::vec3(2.0f, 2.0f, 2.0f));
	teapot.ModelMatrix[0] = glm::translate(glm::mat4(1.0f), glm::vec3(193.0f, 120.0f, 11.0f));

	teapot.material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	teapot.material[0].ambient = glm::vec4(0.19125f, 0.0735f, 0.0225f, 1.0f);
	teapot.material[0].diffuse = glm::vec4(0.7038f, 0.27048f, 0.0828f, 1.0f);
	teapot.material[0].specular = glm::vec4(0.256777f, 0.137622f, 0.086014f, 1.0f);
	teapot.material[0].exponent = 15.0f;
	

}

void set_material_car_nut(void)
{

	set_material_object(&(static_objects[OBJ_TEAPOT]), 0);

}
void set_material_car_wheel(void)
{

	set_material_object(&(static_objects[OBJ_TEAPOT]), 1);
}
void set_material_car_body(void)
{

	set_material_object(&(static_objects[OBJ_TEAPOT]), 2);
}
void set_material_teapot(void)
{

	glUniform4f(loc_material.ambient_color, teapot.material[0].ambient.x, teapot.material[0].ambient.y, teapot.material[0].ambient.z, teapot.material[0].ambient.w);

	glUniform4f(loc_material.diffuse_color, teapot.material[0].diffuse.x, teapot.material[0].diffuse.y, teapot.material[0].diffuse.z, teapot.material[0].diffuse.w);

	glUniform4f(loc_material.emissive_color, teapot.material[0].emission.x, teapot.material[0].emission.y, teapot.material[0].emission.z, teapot.material[0].emission.w);

	glUniform4f(loc_material.ambient_color, teapot.material[0].specular.x, teapot.material[0].specular.y, teapot.material[0].specular.z, teapot.material[0].specular.w);

	glUniform1f(loc_material.specular_exponent, teapot.material[0].exponent);
}
#define rad 1.7f
#define ww 1.0f
void draw_wheel_and_nut(int cam_index) {
	// angle is used in Hierarchical_Car_Correct later
	int i;
	ModelMatrix_CAR_WHEEL = glm::rotate(ModelMatrix_CAR_WHEEL, (rad * car_rotation_angle* 10), glm::vec3(0.0f, 0.0f, 1.0f));

	//glUniform3f(loc_primitive_color, 0.000f, 0.808f, 0.820f); // color name: DarkTurquoise
	set_material_car_wheel();
	draw_geom_obj(GEOM_OBJ_ID_CAR_WHEEL); // draw wheel
	for (i = 0; i < 5; i++) {
		ModelMatrix_CAR_NUT = glm::rotate(ModelMatrix_CAR_WHEEL, TO_RADIAN*72.0f*i, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix_CAR_NUT = glm::translate(ModelMatrix_CAR_NUT, glm::vec3(rad - 0.5f, 0.0f, ww));
		ModelViewProjectionMatrix = ViewProjectionMatrix[cam_index] * ModelMatrix_CAR_NUT;
		//glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1.0, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

		set_material_car_nut();
		//glUniform3f(loc_primitive_color, 0.690f, 0.769f, 0.871f); // color name: LightSteelBlue
		draw_geom_obj(GEOM_OBJ_ID_CAR_NUT); // draw i-th nut
	}
}
void draw_car_dummy(int cam_index) {
	set_material_car_body();
//	glUniform3f(loc_primitive_color, 0.498f, 1.000f, 0.831f); // color name: Aquamarine
	draw_geom_obj(GEOM_OBJ_ID_CAR_BODY); // draw body

	glLineWidth(2.0f);
	draw_axes(cam_index); // draw MC axes of body
	glLineWidth(1.0f);

	ModelMatrix_CAR_DRIVER = glm::translate(ModelMatrix_CAR_BODY, glm::vec3(3.0f, 0.5f, 2.5f));
	ModelMatrix_CAR_DRIVER = glm::rotate(ModelMatrix_CAR_DRIVER, TO_RADIAN*90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix[cam_index]* ModelMatrix_CAR_DRIVER;
	//glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1.0, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	
	glLineWidth(5.0f);
	//draw_axes(cam_index); // draw camera frame at driver seat
	glLineWidth(1.0f);

	ModelMatrix_CAR_WHEEL = glm::translate(ModelMatrix_CAR_BODY, glm::vec3(-3.9f, -3.5f, 4.5f));
	ModelViewProjectionMatrix = ViewProjectionMatrix[cam_index] * ModelMatrix_CAR_WHEEL;
	ModelMatrix_CAR_WHEEL = glm::scale(ModelMatrix_CAR_WHEEL, glm::vec3(1.0f, 1.0f, 1.0f));
	//glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1.0, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	set_material_car_nut();
	draw_wheel_and_nut(cam_index);  // draw wheel 0

	ModelMatrix_CAR_WHEEL = glm::translate(ModelMatrix_CAR_BODY, glm::vec3(3.9f, -3.5f, 4.5f));
	ModelMatrix_CAR_WHEEL = glm::scale(ModelMatrix_CAR_WHEEL, glm::vec3(1.0f, 1.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix[cam_index]* ModelMatrix_CAR_WHEEL;
	//glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1.0, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_wheel_and_nut(cam_index);  // draw wheel 1

	ModelMatrix_CAR_WHEEL = glm::translate(ModelMatrix_CAR_BODY, glm::vec3(-3.9f, -3.5f, -4.5f));
	ModelMatrix_CAR_WHEEL = glm::scale(ModelMatrix_CAR_WHEEL, glm::vec3(1.0f, 1.0f, -1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix[cam_index] * ModelMatrix_CAR_WHEEL;
	//glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1.0, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_wheel_and_nut(cam_index);  // draw wheel 2

	ModelMatrix_CAR_WHEEL = glm::translate(ModelMatrix_CAR_BODY, glm::vec3(3.9f, -3.5f, -4.5f));
	ModelMatrix_CAR_WHEEL = glm::scale(ModelMatrix_CAR_WHEEL, glm::vec3(1.0f, 1.0f, -1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix[cam_index]* ModelMatrix_CAR_WHEEL;
	//glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1.0, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_wheel_and_nut(cam_index);  // draw wheel 3
}

void draw_tiger(void)
{
	glFrontFace(GL_CW);
	glBindVertexArray(tiger_VAO);
	glDrawArrays(GL_TRIANGLES, tiger_vertex_offset[tiger_data.cur_frame], 3 * tiger_n_triangles[tiger_data.cur_frame]);
	glBindVertexArray(0);
}

void draw_animated_tiger(int cam_index) {

	
	tigerHead = ModelViewMatrix;

	if (tiger_time < 15)
	{
		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		//ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], -tiger_data.rotation_angle, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(40.0f, 40.0f - tiger_time, 5.0f));

	}
	else if (tiger_time >= 15 && tiger_time < 25) // first conrner
	{
		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		//ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], -tiger_data.rotation_angle, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(40.0f, 25.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, ((tiger_time - 15)* 9)* TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (tiger_time >= 25 && tiger_time<50)
	{
		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		//ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(20.0f + tiger_time , 30.0f, 5.0f));
	//	ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3( 30.0f, 20.0f + tiger_time, 5.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(40.0f + tiger_time - 25.0f , 25.0f, 5.0f));

		ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (tiger_time >= 50 && tiger_time < 60) // second corner
	{

		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		//ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], -tiger_data.rotation_angle, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(65.0f, 25.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (90.0f - ((tiger_time - 50)) * -9)* TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 60 && tiger_time < 110)
	{
		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(65.0f, 25.0f + tiger_time - 60.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -180.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 110 && tiger_time < 120) // third corner
	{
		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(65.0f,75.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (-180.0f +((tiger_time-110)* 9))*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 120 && tiger_time < 160)
	{
		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(65.0f - (tiger_time - 120) , 75.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 160 && tiger_time < 170) // fourth corner
	{
		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(25.0f, 75.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (-90.0f + ((tiger_time-160) * -9 )) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 170 && tiger_time < 230)
	{

		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(25.0f, 75.0f + (tiger_time - 170), 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -180.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	} 
	else if (tiger_time >= 230 && tiger_time < 240) // fifth corner, rotating 90 degrees 
	{

		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(25.0f, 135.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (-180.0f + ((tiger_time - 230) * -9)) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 240 && tiger_time < 260)
	{

		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(25.0f + tiger_time - 240.0f , 135.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (tiger_time >= 260 && tiger_time < 280) // 180 degree
	{
		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(45.0f, 135.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (90.0f + ((tiger_time - 260) * -9)) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 280 && tiger_time < 300)
	{

		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(45.0f - tiger_time + 280.0f, 135.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 300 && tiger_time < 310)
	{
		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(25.0f, 135.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (-90.0f + ((tiger_time - 300) * 9)) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 310 && tiger_time < 360)
	{

		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(25.0f, 135.0f - (tiger_time - 310.0f), 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 0.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 360 && tiger_time < 370)
	{
		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(25.0f, 85.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (((tiger_time - 360) * 9)) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 370 && tiger_time < 470)
	{

		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(25.0f + (tiger_time - 370.0f), 85.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

		
	}
	else if (tiger_time >= 470 && tiger_time < 480)
	{

		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(125.0f, 85.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (90.0f -((tiger_time - 470) * 9)) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 480 && tiger_time < 500)
	{

		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(125.0f, 85.0f - (tiger_time - 480.0f), 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 0.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));


	}
	else if (tiger_time >= 500 && tiger_time < 510)
	{

		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(125.0f, 65.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (((tiger_time - 500) * -9)) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 510 && tiger_time < 520)
	{

		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(125.0f - (tiger_time - 510.0f), 65.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 520 && tiger_time < 530)
	{
		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(115.0f, 65.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (-90 + ((tiger_time - 520) * 9)) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 530 && tiger_time < 570)
	{

		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(115.0f, 65.0f - (tiger_time-530.0f), 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 0.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 570 && tiger_time < 580)
	{

		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(115.0f, 25.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (((tiger_time - 570) * 9)) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 580 && tiger_time < 610)
	{

		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(115.0f + (tiger_time - 580.0f), 25.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 610 && tiger_time < 630)
	{

		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(145.0f, 25.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (90 + ((tiger_time - 610) * 9)) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 630 && tiger_time < 660)
	{

		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(145.0f -(tiger_time-630.0f), 25.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 270.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 660 && tiger_time < 670)
	{

		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(115.0f, 25.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (270 - ((tiger_time - 660) * 9)) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 670 && tiger_time < 720)
	{
		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(115.0f, 25.0f + (tiger_time-670.0f), 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 180.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 720 && tiger_time < 730)
	{
		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(115.0f, 75.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (180 - ((tiger_time - 720) * 9)) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 730 && tiger_time < 740)
	{

		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(115.0f + tiger_time - 730.0f , 75.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 740 && tiger_time < 750)
	{

		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(125.0f, 75.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (90 + ((tiger_time - 740) * 9)) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 750 && tiger_time < 770)
	{

		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(125.0f, 75.0f + tiger_time-750.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 180.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 770 && tiger_time < 780)
	{

		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(125.0f, 95.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, ((180.0f - (tiger_time - 770) * 9)) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 780 && tiger_time < 820)
	{

		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(125.0f + (tiger_time - 780.0f) ,  95.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 820 && tiger_time < 830)
	{

		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(165.0f, 95.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, ((90.0f + (tiger_time - 820) * 9)) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 830 && tiger_time < 880)
	{

		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(165.0f, 95.0f +(tiger_time - 830.0f), 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 180.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 880 && tiger_time < 890)
	{

		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(165.0f, 145.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, ((180.0f + (tiger_time - 880) * 9)) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 890 && tiger_time < 960)
	{
		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(165.0f - (tiger_time-890.0f), 145.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 270.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 960 && tiger_time < 980)
	{

		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(95.0f, 145.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, ((270.0f - (tiger_time - 960) * 9)) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 980 && tiger_time < 1050)
	{

		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(95.0f + (tiger_time - 980.0f), 145.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 1050 && tiger_time < 1060)
	{

		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(165.0f, 145.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, ((90.0f - (tiger_time - 1050) * 9)) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 1060 && tiger_time < 1100)
	{

		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(165.0f, 145.0f - (tiger_time - 1060.0f), 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 0.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 1100 && tiger_time < 1110)
	{

		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(165.0f, 105.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (((tiger_time - 1100) * 9)) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 1110 && tiger_time < 1160)
	{

		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(165.0f +(tiger_time-1110.0f), 105.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 1160 && tiger_time < 1170)
	{

		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(215.0f, 105.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, ((90.0f - (tiger_time - 1160) * 9)) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 1170 && tiger_time < 1200)
	{

		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(215.0f, 105.0f -(tiger_time-1170.0f), 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 0.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 1200 && tiger_time < 1210)
	{

		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(215.0f, 75.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, ((0.0f - (tiger_time - 1200) * 9)) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 1210 && tiger_time < 1250)
	{


		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(215.0f - (tiger_time - 1210.0f), 75.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 1250 && tiger_time < 1260)
	{

		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(185.0f, 75.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, ((-90.0f + (tiger_time - 1250) * 9)) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 1260 && tiger_time< 1310)
	{


		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(185.0f, 75.0f - (tiger_time - 1260), 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 0.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (tiger_time >= 1310 && tiger_time < 1320)
	{

		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(185.0f, 25.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (( (tiger_time - 1310) * 9)) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_time >= 1320 && tiger_time < 1340)
	{

		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(185.0f + (tiger_time - 1320.0f), 25.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (tiger_time >= 1340)
	{

		ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(205.0f, 25.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, ((90.0f + (tiger_time - 1340) * 9)) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}


		ModelViewMatrix *= tiger[tiger_data.cur_frame].ModelMatrix[0];
	
		ModelViewProjectionMatrix = ProjectionMatrix[cam_index] * ModelViewMatrix;
	//	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1.0, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

		draw_tiger();
}

int initial2 = 1;
void draw_animated_teapot(int cam_index) {
	

	

	if (initial2)
	{
		temp = ModelViewMatrix;
		temp = glm::translate(temp, glm::vec3(100.0f, 100.0f, 30.0f));

		initial2 = 0;
	}


	//ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(100.0f, 100.0f, 1.0f));
//	ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], -teapot_rotation_angle, glm::vec3(0.0f, 0.0f, 1.0f));
	//ModelViewMatrix = glm::rotate(ModelViewMatrix, -teapot_rotation_angle, glm::vec3(0.0f, 0.0f, 1.0f));
	//ModelViewMatrix = glm::rotate(ViewMatrix[cam_index], -teapot_rotation_angle, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::translate(ViewMatrix[cam_index], glm::vec3(130.0f, 130.0f, 1.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -teapot_rotation_angle, glm::vec3(0.0f, 0.0f, 1.0f));

	ModelViewMatrix = glm::scale(ModelViewMatrix,
		glm::vec3(2.0f, 2.0f, 2.0f));

	ModelViewProjectionMatrix = ProjectionMatrix[cam_index] * ModelViewMatrix;
//	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1.0, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

	//glUniform3f(loc_primitive_color,teapot.material[0].diffuse.r,
		//teapot.material[0].diffuse.g, teapot.material[0].diffuse.b);

	glBindVertexArray(teapot.VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * teapot.n_triangles);
	glBindVertexArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3* teapot.n_triangles, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(1);

	ModelViewProjectionMatrix = glm::scale(ModelViewProjectionMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);


}
int initial = 1;
void render_car(int cam_index)
{
	if (initial)
	{
		temp = ModelViewMatrix;
		//temp = glm::translate(temp, glm::vec3(170.0f, 50.0f, 30.0f));
		temp = glm::translate(temp, glm::vec3(100.0f, 100.0f, 30.0f));

		initial = 0;
	}
	
		ModelMatrix_CAR_BODY = glm::rotate(temp, -tiger_data.rotation_angle, glm::vec3(0.0f, 1.0f, 0.0f));
	
		ModelMatrix_CAR_BODY = glm::translate(ModelMatrix_CAR_BODY, glm::vec3(120.0f, 0.0f, 0.0f));
		//ModelMatrix_CAR_BODY = glm::rotate(ModelMatrix_CAR_BODY, 90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
		ModelMatrix_CAR_BODY = glm::rotate(ModelMatrix_CAR_BODY, 90.0f*TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));


	//ModelMatrix_CAR_BODY = glm::rotate(glm::mat4(1.0f), 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));

	ModelViewProjectionMatrix = ViewProjectionMatrix[cam_index] * ModelMatrix_CAR_BODY;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1.0, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_car_dummy(cam_index);

	ModelViewProjectionMatrix = glm::scale(ViewProjectionMatrix[cam_index], glm::vec3(5.0f, 5.0f, 5.0f));
	//glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1.0, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	
	glLineWidth(2.0f);
	
	ModelViewProjectionMatrix = ViewProjectionMatrix[cam_index] * ViewMatrix[cam_index];
	//glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1.0, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

}
void cleanup_OpenGL_stuffs(void) {
	for (int i = 0; i < n_static_objects; i++) {
		glDeleteVertexArrays(1, &(static_objects[i].VAO));
		glDeleteBuffers(1, &(static_objects[i].VBO));
	}

	for (int i = 0; i < N_TIGER_FRAMES; i++) {
		glDeleteVertexArrays(1, &(tiger[i].VAO));
		glDeleteBuffers(1, &(tiger[i].VBO));
	}

	glDeleteVertexArrays(1, &VAO_axes);
	glDeleteBuffers(1, &VBO_axes);
}
