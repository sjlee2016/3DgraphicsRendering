
void display_camera(int cam_index) {

	
	// should optimize this dispaly function further to reduce the amount of floating-point operations.
	glm::mat4 Matrix_TIGER_tmp;
	glViewport(viewport[cam_index].x, viewport[cam_index].y, viewport[cam_index].w, viewport[cam_index].h);

	ModelViewProjectionMatrix = glm::scale(ViewProjectionMatrix[cam_index], glm::vec3(1.0f, 1.0f, 1.0f));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glLineWidth(2.0f);
	//draw_axes(cam_index);
	glLineWidth(1.0f);
	if (flag2)
	{
		set_ViewMatrix_for_driver(8);
	}
	//	else if (view_mode == VIEW_TIGER) set_ViewMatrix_for_tiger(8);

	//draw_view(cam_index);

	
	glUseProgram(h_ShaderProgram_PS);
	
	//render_car(cam_index);
	//set_material_floor();
	//raw_floor(cam_index);


	//ModelViewMatrix = glm::translate(ViewMatrix[cam_index], glm::vec3(30.0f, 60.0f, 0.0f));
	//ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(200.0f, 100.0f, 100.0f));
	//ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	//ModelViewProjectionMatrix = ProjectionMatrix[cam_index] * ModelViewMatrix;
	//ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));

    //draw_static_object2(&(static_objects[OBJ_COW]), 0, cam_index);

	//set_material_object(&(static_objects[OBJ_BUILDING]), 0);
	set_material_floor();
	draw_static_object(&(static_objects[OBJ_BUILDING]), 0, cam_index);
	set_material_object(&(static_objects[OBJ_COW]), 0);
	draw_static_object(&(static_objects[OBJ_COW]), 0, cam_index);
	set_material_teapot();
	draw_animated_teapot(cam_index);

	set_material_object(&(static_objects[OBJ_COW]), 1);
	draw_static_object(&(static_objects[OBJ_COW]), 1, cam_index);
	set_material_object(&(static_objects[OBJ_TABLE]), 0);
	draw_static_object(&(static_objects[OBJ_TABLE]), 0, cam_index);
	set_material_object(&(static_objects[OBJ_TABLE]), 1);

	draw_static_object(&(static_objects[OBJ_TABLE]), 1, cam_index);

	set_material_object(&(static_objects[OBJ_LIGHT]), 0);
	draw_static_object(&(static_objects[OBJ_LIGHT]), 0, cam_index);

	set_material_object(&(static_objects[OBJ_LIGHT]), 1);
	draw_static_object(&(static_objects[OBJ_LIGHT]), 1, cam_index);

	set_material_object(&(static_objects[OBJ_LIGHT]), 2);
	draw_static_object(&(static_objects[OBJ_LIGHT]), 2, cam_index);

	set_material_object(&(static_objects[OBJ_LIGHT]), 3);
	draw_static_object(&(static_objects[OBJ_LIGHT]), 3, cam_index);

	set_material_object(&(static_objects[OBJ_LIGHT]), 4);
	draw_static_object(&(static_objects[OBJ_LIGHT]), 4, cam_index);

	set_material_object(&(static_objects[OBJ_TEAPOT]), 0);
	draw_static_object(&(static_objects[OBJ_TEAPOT]), 0, cam_index);

	set_material_object(&(static_objects[OBJ_TEAPOT]), 1);
	draw_static_object(&(static_objects[OBJ_TEAPOT]), 1, cam_index);


	set_material_object(&(static_objects[OBJ_TEAPOT]), 2);
	draw_static_object(&(static_objects[OBJ_TEAPOT]), 2, cam_index);


	set_material_object(&(static_objects[OBJ_NEW_CHAIR]), 0);
	draw_static_object(&(static_objects[OBJ_NEW_CHAIR]), 0, cam_index);


	set_material_object(&(static_objects[OBJ_NEW_CHAIR]), 1);
	draw_static_object(&(static_objects[OBJ_NEW_CHAIR]), 1, cam_index);

	set_material_object(&(static_objects[OBJ_FRAME]), 0);
	draw_static_object(&(static_objects[OBJ_FRAME]), 0, cam_index);

	set_material_object(&(static_objects[OBJ_SPIDER]), 0);
	draw_static_object(&(static_objects[OBJ_SPIDER]), 0, cam_index);

	set_material_tiger();
	draw_animated_tiger(cam_index);
	//	draw_static_object(&(static_objects[OBJ_COW]), 0, cam_index);



}
void display_tiger(int cam_index)
{
	glm::mat4 Matrix_TIGER_tmp;

	glViewport(viewport[cam_index].x, viewport[cam_index].y, viewport[cam_index].w, viewport[cam_index].h);

	ModelViewProjectionMatrix = glm::scale(ViewProjectionMatrix[cam_index], glm::vec3(1.0f, 1.0f, 1.0f));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glLineWidth(2.0f);
	draw_animated_tiger(cam_index);
}