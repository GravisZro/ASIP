#include "read_ply_file.h"
#define TINYPLY_IMPLEMENTATION 1
#include "tinyply.h"

using namespace tinyply;

void manual_timer::start() { t0 = std::chrono::high_resolution_clock::now(); }
void manual_timer::stop() { timestamp = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - t0).count() * 1000; }
const double & manual_timer::get() { return timestamp; }


bool read_ply_file(const std::string & filepath, vector<float3> &_vertices,
	vector<std::pair<uint, uint>> &_edges, vector<vector<uint>>& _polygons)
{
	try
	{
		std::ifstream ss(filepath, std::ios::binary);
		if (ss.fail()) throw std::runtime_error("failed to open " + filepath);

		PlyFile file;
		file.parse_header(ss);

		std::cout << "........................................................................\n";
		for (auto c : file.get_comments()) std::cout << "Comment: " << c << std::endl;
		for (auto e : file.get_elements())
		{
			std::cout << "element - " << e.name << " (" << e.size << ")" << std::endl;
			for (auto p : e.properties) std::cout << "\tproperty - " << p.name << " (" << tinyply::PropertyTable[p.propertyType].str << ")" << std::endl;
		}
		std::cout << "........................................................................\n";

		// Tinyply treats parsed data as untyped byte buffers. See below for examples.
		std::shared_ptr<PlyData> vertices, normals, edges, faces, texcoords;

		// The header information can be used to programmatically extract properties on elements
		// known to exist in the header prior to reading the data. For brevity of this sample, properties 
		// like vertex position are hard-coded: 
		try { vertices = file.request_properties_from_element("vertex", { "x", "y", "z" }); }
		catch (const std::exception & e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

		try { normals = file.request_properties_from_element("vertex", { "nx", "ny", "nz" }); }
		catch (const std::exception & e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

		try { texcoords = file.request_properties_from_element("vertex", { "u", "v" }); }
		catch (const std::exception & e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

		try { edges = file.request_properties_from_element("edge", { "vertex1", "vertex2" }); }
		catch (const std::exception & e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

		// Providing a list size hint (the last argument) is a 2x performance improvement. If you have 
		// arbitrary ply files, it is best to leave this 0. 
		try { faces = file.request_properties_from_element("face", { "vertex_indices" }, 0); }
		catch (const std::exception & e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

		manual_timer read_timer;

		read_timer.start();
		file.read(ss);
		read_timer.stop();

		std::cout << "Reading took " << read_timer.get() / 1000.f << " seconds." << std::endl;
		if (vertices) std::cout << "\tRead " << vertices->count << " total vertices " << std::endl;
		if (normals) std::cout << "\tRead " << normals->count << " total vertex normals " << std::endl;
		if (texcoords) std::cout << "\tRead " << texcoords->count << " total vertex texcoords " << std::endl;
		if (edges) std::cout << "\tRead " << edges->count << " total edges " << std::endl;
		if (faces) std::cout << "\tRead " << faces->count << " total faces (triangles) " << std::endl;

		// type casting to your own native types
		const size_t numVerticesBytes = vertices->buffer.size_bytes();
		_vertices.resize(vertices->count);
		std::memcpy(_vertices.data(), vertices->buffer.get(), numVerticesBytes);

		if (edges) {
			const size_t numEdgesBytes = edges->buffer.size_bytes();
			_edges.resize(edges->count);
			//std::cout << "edges->t = " << uint(edges->t) << std::endl;
			std::memcpy(_edges.data(), edges->buffer.get(), numEdgesBytes);
		}

		if (faces) {
			const size_t numFacesBytes = faces->buffer.size_bytes();
			_polygons.resize(faces->count);
			assert(faces->t == tinyply::Type::UINT32);
			std::memcpy(_polygons.data(), faces->buffer.get(), numFacesBytes);
		}

		return true;

	}
	catch (const std::exception & e)
	{
		std::cerr << "Caught tinyply exception: " << e.what() << std::endl;
	}
	return false;
}