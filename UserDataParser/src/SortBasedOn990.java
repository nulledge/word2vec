import java.util.Comparator;

import org.json.simple.JSONObject;

public class SortBasedOn990 implements Comparator<JSONObject> {
    /*
    * (non-Javadoc)
    * 
    * @see java.util.Comparator#compare(java.lang.Object, java.lang.Object)
    * lhs- 1st message in the form of json object. rhs- 2nd message in the form
    * of json object.
    */
    @Override
    public int compare(JSONObject lhs, JSONObject rhs) {
        return getFloat(lhs, "990") > getFloat(rhs, "990") ? 1 : 
			(getFloat(lhs, "990") < getFloat(rhs, "990") ? -1 : 0);
    }
    
    private float getFloat(JSONObject obj, String key) {
    	return Float.parseFloat(obj.get(key).toString());
    }
}